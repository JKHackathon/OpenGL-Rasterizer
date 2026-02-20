#include "obj_loader.hpp"

// Converts string to float (throws exception on failure)
float string_to_float(std::string_view value_view) {
    float value;
    auto [ptr, ec] = std::from_chars(
        value_view.data(), value_view.data() + value_view.size(), value);
    if (ec == std::errc::invalid_argument) {
        throw std::runtime_error(std::string(value_view) +
                                 " is not a float.\n");
    } else if (ec == std::errc::result_out_of_range) {
        throw std::runtime_error("This number is larger than a float.\n");
    }
    return value;
}

// Converts string to float (throws exception on failure)
int string_to_int(std::string_view value_view) {
    int value;
    auto [ptr, ec] = std::from_chars(
        value_view.data(), value_view.data() + value_view.size(), value);
    if (ec == std::errc::invalid_argument) {
        throw std::runtime_error(std::string(value_view) +
                                 " is not an integer.\n");
    } else if (ec == std::errc::result_out_of_range) {
        throw std::runtime_error("This number is larger than an integer.\n");
    }
    return value;
}

std::tuple<int, std::optional<int>, std::optional<int>>
ObjLoader::get_face_vertex_index(std::string_view face_index_group) {
    std::vector<std::string_view> tokens;

    size_t start = 0;
    while (true) {
        size_t pos = face_index_group.find('/', start);
        if (pos == std::string_view::npos) {
            tokens.emplace_back(face_index_group.substr(start));
            break;
        }
        tokens.emplace_back(face_index_group.substr(start, pos - start));
        start = pos + 1;
    }

    int v = string_to_int(tokens[0]);  // TODO: use from_chars instead

    std::optional<int> vt;
    std::optional<int> vn;
    if (tokens[1] != "") vt = string_to_int(tokens[1]);
    if (tokens[2] != "") vn = string_to_int(tokens[2]);
    return {v, vt, vn};
}

void ObjLoader::parse_obj_file(const char* filename) {
    std::ifstream file;
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + std::string(filename) + "\n");
    }

    fprintf(stdout, "Using obj file: %s\n", std::string(filename).c_str());

    std::ofstream testFile;
    testFile.open("test_file.obj");
    if (!testFile.is_open()) {
        throw std::runtime_error("Failed to open test file: \n");
    }

    std::string curr_material;
    std::string filepath_dir =
        std::filesystem::path(filename).parent_path().string();
    if (!filepath_dir.empty()) {
        filepath_dir +=
            "/";  // TODO: this expects forward slash for dir structure
    }
    std::string line;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {  // good, but prob not necessary
            continue;
        }

        std::string_view line_view = line;
        size_t pos = line_view.find(' ', 0);
        std::string_view type = line_view.substr(0, pos);
        std::vector<std::string_view> tokens;

        size_t start = pos + 1;
        while (true) {
            pos = line_view.find(' ', start);
            if (pos == std::string_view::npos) {
                auto last_val = line_view.substr(start);
                if (last_val != "") {
                    tokens.emplace_back(last_val);
                }
                break;
            }
            auto next_val = line_view.substr(start, pos - start);
            if (next_val != "") {
                tokens.emplace_back(next_val);
            }
            start = pos + 1;
        }

        // if (type == "g") {
        //     auto new_group_u = std::make_unique<Group>();
        //     curr_mesh = new_group_u.get();
        //     parser.groups.emplace(data[1], std::move(new_group_u));
        //     continue;
        // }
        if (type == "v") {
            vertices.emplace_back(glm::vec3(string_to_float(tokens[0]),
                                            string_to_float(tokens[1]),
                                            string_to_float(tokens[2])));
            auto vstring = "v  " + std::string(tokens[0]) + " " +
                           std::string(tokens[1]) + " " +
                           std::string(tokens[2]) + "\n";
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "vn") {
            vertex_normals.emplace_back(glm::vec3(string_to_float(tokens[0]),
                                                  string_to_float(tokens[1]),
                                                  string_to_float(tokens[2])));
            auto vstring = "vn " + std::string(tokens[0]) + " " +
                           std::string(tokens[1]) + " " +
                           std::string(tokens[2]) + "\n";
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "vt") {
            // TODO: only u is required, v,w optional
            vertex_textures.emplace_back(glm::vec2(string_to_float(tokens[0]),
                                                   string_to_float(tokens[1])));
            auto vstring = "vt " + std::string(tokens[0]) + " " +
                           std::string(tokens[1]) + "\n";
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "f") {
            if (tokens.size() > 3) {  // polygon
                std::vector<unsigned int> positions;
                std::vector<unsigned int> normals;
                std::vector<unsigned int> texcoords;

                std::string string = "f ";
                for (int i = 0; i < tokens.size(); i++) {
                    const auto [v, t, n] = get_face_vertex_index(tokens[i]);
                    positions.push_back(v);
                    if (t.has_value()) {
                        texcoords.push_back(t.value());
                    }
                    if (n.has_value()) {
                        normals.push_back(n.value());
                    }

                    string += std::format("{}/{}/{} ", v, t.value(), n.value());
                }
                string += "\n";
                testFile.write(string.c_str(), string.size());
                // auto vstring = "f " + std::string(tokens[0]) +
                //                std::string(tokens[1]) +
                //                std::string(tokens[2]) +
                //                "\n";

                for (int i = 1; i < positions.size() - 1; i++) {
                    Face f;
                    f.vertex_indices =
                        glm::ivec3(positions[0] - 1, positions[i] - 1,
                                   positions[i + 1] - 1);
                    if (!normals.empty()) {
                        f.vertex_texture_indices =
                            glm::ivec3(texcoords[0] - 1, texcoords[i] - 1,
                                       texcoords[i + 1] - 1);
                        f.vertex_normal_indices = glm::ivec3(
                            normals[0] - 1, normals[i] - 1, normals[i + 1] - 1);
                    }
                    if (!curr_material.empty()) {
                        f.material = curr_material;
                    }
                    faces.emplace_back(f);
                }
                continue;
            }

            Face f;
            const auto [v1, vt1, vn1] = get_face_vertex_index(tokens[0]);
            const auto [v2, vt2, vn2] = get_face_vertex_index(tokens[1]);
            const auto [v3, vt3, vn3] = get_face_vertex_index(tokens[2]);

            // Convert to 0-based indices
            f.vertex_indices = glm::ivec3(v1 - 1, v2 - 1, v3 - 1);
            if (vt1.has_value()) {
                f.vertex_texture_indices = glm::ivec3(
                    vt1.value() - 1, vt2.value() - 1, vt3.value() - 1);
            }
            if (vn1.has_value()) {
                f.vertex_normal_indices = glm::ivec3(
                    vn1.value() - 1, vn2.value() - 1, vn3.value() - 1);
            }
            if (!curr_material.empty()) {
                f.material = curr_material;
            }
            faces.emplace_back(f);

            auto string = std::format(
                "f {}/{}/{} {}/{}/{} {}/{}/{} \n", v1, vt1.value(), vn1.value(),
                v2, vt2.value(), vn2.value(), v3, vt3.value(), vn3.value());
            testFile.write(string.c_str(), string.size());
            continue;
        }

        if (type == "mtllib") {
            auto mtl_filename = filepath_dir + std::string(tokens[0]);
            parse_mtl_file(filepath_dir, mtl_filename);
            loaded_materials.emplace(mtl_filename);
        }

        if (type == "usemtl") {
            auto material_name = std::string(tokens[0]);
            if (!materials.contains(material_name)) {
                throw std::runtime_error("Material '" + material_name +
                                         "' not found!\n");
            }
            curr_material = material_name;
        }

        // TODO: how do i want to handle objects/groups? I will likely ignore
        // groups, for now, ignore objects
        // TODO: smooth shading
    }
}

void ObjLoader::parse_mtl_file(std::string filepath_dir,
                               std::string mtl_filename) {
    if (loaded_materials.contains(filepath_dir +
                                  mtl_filename)) {  // File already loaded
        return;
    }

    std::ifstream file;
    file.open(filepath_dir + mtl_filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open mtl file: " +
                                 std::string(filepath_dir + mtl_filename) +
                                 "\n");
    }

    Material* curr_material;

    std::string line;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::string_view line_view = line;
        std::vector<std::string_view> tokens;

        size_t start = 0;
        while (true) {
            size_t pos = line_view.find(' ', start);
            if (pos == std::string_view::npos) {
                auto last_val = line_view.substr(start);
                if (last_val != "") {
                    tokens.emplace_back(last_val);
                }
                break;
            }
            auto next_val = line_view.substr(start, pos - start);
            if (next_val != "") {
                if (next_val[0] == '\t') {
                    next_val = next_val.substr(1);
                }
                tokens.emplace_back(next_val);
            }
            start = pos + 1;
        }

        if (tokens.empty()) {
            continue;
        }

        auto type = tokens[0];
        if (type == "newmtl") {
            auto mat_u = std::make_unique<Material>();
            curr_material = mat_u.get();
            materials.emplace(std::string(tokens[1]), std::move(mat_u));
        }

        else if (type == "Ka") {  // ambient
            curr_material->K_a = glm::vec3(string_to_float(tokens[1]),
                                           string_to_float(tokens[2]),
                                           string_to_float(tokens[3]));
        } else if (type == "Kd") {  // diffuse
            curr_material->K_d = glm::vec3(string_to_float(tokens[1]),
                                           string_to_float(tokens[2]),
                                           string_to_float(tokens[3]));
        } else if (type == "Ks") {  // specular
            curr_material->K_s = glm::vec3(string_to_float(tokens[1]),
                                           string_to_float(tokens[2]),
                                           string_to_float(tokens[3]));
        } else if (type == "Ns") {  // shininess
            curr_material->shininess = string_to_float(tokens[1]);
        }
        // Ke: emission

        // Transparency
        else if (type == "d") {  // dissolve
            curr_material->transparency = 1 - string_to_float(tokens[1]);
        } else if (type == "Tr") {  // transmission
            curr_material->transparency = string_to_float(tokens[1]);
        } else if (type == "Tf") {  // transmission filter color
            // TODO: can be CIEXYZ, or spectral curve file
            curr_material->transmission_color = glm::vec3(
                string_to_float(tokens[1]), string_to_float(tokens[2]),
                string_to_float(tokens[3]));
        } else if (type == "Ni") {  // optical density/index of refraction
            curr_material->ior = string_to_float(tokens[1]);
        }

        // TODO: may need to handle this
        // else if (type == "illum") { // illumination models

        // }

        // TODO: make neater
        // TODO: test this works as intended
        // Texture Maps
        else if (type == "map_Ka") {  // ambient map
            auto texture_file = filepath_dir + std::string(tokens[1]);
            if (loaded_texture_maps.contains(texture_file)) {
                curr_material->ambient_map_filepath =
                    texture_maps.at(texture_file);
                continue;
            }
            curr_material->ambient_map_filepath =
                std::make_shared<TextureMap>();
            decode_texture_png(texture_file,
                               curr_material->ambient_map_filepath.get());
            texture_maps.emplace(texture_file,
                                 curr_material->ambient_map_filepath);
            loaded_texture_maps.emplace(texture_file);

        } else if (type == "map_Kd") {  // diffuse map
            auto texture_file = filepath_dir + std::string(tokens[1]);
            if (loaded_texture_maps.contains(texture_file)) {
                curr_material->diffuse_map_filepath =
                    texture_maps.at(texture_file);
                continue;
            }
            curr_material->diffuse_map_filepath =
                std::make_shared<TextureMap>();
            decode_texture_png(texture_file,
                               curr_material->diffuse_map_filepath.get());
            texture_maps.emplace(texture_file,
                                 curr_material->diffuse_map_filepath);
            loaded_texture_maps.emplace(texture_file);

        } else if (type == "map_Ks") {  // specular map
            auto texture_file = filepath_dir + std::string(tokens[1]);
            if (loaded_texture_maps.contains(texture_file)) {
                curr_material->specular_map_filepath =
                    texture_maps.at(texture_file);
                continue;
            }
            curr_material->specular_map_filepath =
                std::make_shared<TextureMap>();
            decode_texture_png(texture_file,
                               curr_material->specular_map_filepath.get());
            texture_maps.emplace(texture_file,
                                 curr_material->specular_map_filepath);
            loaded_texture_maps.emplace(texture_file);

        } else if (type == "map_bump" || type == "bump") {  // bump map
            auto texture_file = filepath_dir + std::string(tokens[1]);
            if (loaded_texture_maps.contains(texture_file)) {
                curr_material->bump_map_filepath =
                    texture_maps.at(texture_file);
                continue;
            }
            curr_material->bump_map_filepath = std::make_shared<TextureMap>();
            decode_texture_png(texture_file,
                               curr_material->bump_map_filepath.get());
            texture_maps.emplace(texture_file,
                                 curr_material->bump_map_filepath);
            loaded_texture_maps.emplace(texture_file);
        }
    }
}

void ObjLoader::decode_texture_png(std::string filename,
                                   TextureMap* textureMap) {
    unsigned int error = lodepng::decode(textureMap->pixels, textureMap->width,
                                         textureMap->height, filename);
    if (error) {
        throw std::runtime_error("Decoder error for " + filename + ": " +
                                 std::string(lodepng_error_text(error)));
    }
}