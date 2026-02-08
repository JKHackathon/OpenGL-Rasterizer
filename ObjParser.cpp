#include "ObjParser.hpp"

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
ObjParser::get_face_vertex_index(std::string_view face_index_group) {
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

void ObjParser::parse_obj_file(const char* filename) {
    std::ifstream file;
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + std::string(filename) + "\n");
    }

    std::ofstream testFile;
    testFile.open("test_file.obj");
    if (!testFile.is_open()) {
        throw std::runtime_error(
            "Failed to open test file: \n");
    }

    std::string line;
    size_t line_num = 0;
    while (getline(file, line)) {
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
            auto vstring = "v" + std::string(tokens[0]) + std::string(tokens[1]) + std::string(tokens[2]);
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "vn") {
            vertex_normals.emplace_back(glm::vec3(string_to_float(tokens[0]),
                                                  string_to_float(tokens[1]),
                                                  string_to_float(tokens[2])));
            auto vstring = "vn" + std::string(tokens[0]) + std::string(tokens[1]) + std::string(tokens[2]);
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "vt") {
            vertex_textures.emplace_back(glm::vec3(string_to_float(tokens[0]),
                                                   string_to_float(tokens[1]),
                                                   string_to_float(tokens[2])));
            auto vstring = "vt" + std::string(tokens[0]) + std::string(tokens[1]) + std::string(tokens[2]);
            testFile.write(vstring.c_str(), vstring.size());
            continue;
        }

        if (type == "f") {
            if (tokens.size() > 3) {  // polygon
                std::vector<unsigned int> positions;
                std::vector<unsigned int> normals;
                std::vector<unsigned int> texcoords;

                for (int i = 0; i < tokens.size(); i++) {
                    const auto [v, t, n] = get_face_vertex_index(tokens[i]);
                    positions.push_back(v);
                    if (t.has_value()) {
                        texcoords.push_back(t.value());
                    }
                    if (n.has_value()) {
                        normals.push_back(n.value());
                    }
                }
                for (int i = 1; i < positions.size() - 1; i++) {
                    Face f;
                    f.vertex_indices = glm::ivec3(positions[0], positions[i],
                                                  positions[i + 1]);
                    if (!normals.empty()) {
                        f.vertex_texture_indices = glm::ivec3(
                            texcoords[0], texcoords[i], texcoords[i + 1]);
                        f.vertex_normal_indices =
                            glm::ivec3(normals[0], normals[i], normals[i + 1]);
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
            faces.emplace_back(f);
        }
        line_num++;
    }
}