#include "rasterizer.hpp"

void Rasterizer::bindProgram(GLuint program) {
    if (program == curr_state.boundProgram) {
        return;
    }
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        throw std::runtime_error("Program not yet linked\n");
    }
    glUseProgram(program);
    curr_state.boundProgram = program;
}

void Rasterizer::bindVAO(GLuint vao) {
    if (curr_state.boundVAO == vao) {
        return;
    }
    glBindVertexArray(vao);
    curr_state.boundVAO = vao;
}

// TODO: generally a vbo, but not always
void Rasterizer::bindArrayBuffer(GLuint vbo) {
    if (curr_state.boundArrayBuffer == vbo) {
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    curr_state.boundArrayBuffer = vbo;
}

void Rasterizer::bindElementBuffer(GLuint ebo) {
    if (curr_state.boundElementBuffer == ebo) {
        return;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    curr_state.boundElementBuffer = ebo;
}

// TODO: allow for one mesh to have multiple materials
void Rasterizer::uploadMesh(Mesh& mesh) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    bindVAO(vao);
    GLuint vbo;
    glGenBuffers(1, &vbo);
    bindArrayBuffer(vbo);

    // Set up unified vertex buffer
    std::vector<VertexData> vertices;
    vertices.reserve(mesh.positions.size() * 3);
    for (int i = 0; i < mesh.positions.size(); i++) {
        vertices.emplace_back(mesh.positions[i], mesh.normals[i],
                              mesh.texcoords[i]);
        // TODO: for some reason, this is not supposed to be normalized???
        // Expects UV not ST coords
        // glm::normalize(mesh.texcoords[i]));  // Change to st-coords
    }

    // For drawing without EBO
    // for (auto& triangle : mesh.triangles) {
    //     vertices.push_back(mesh.positions[triangle.vertices.x]);
    //     vertices.push_back(mesh.positions[triangle.vertices.y]);
    //     vertices.push_back(mesh.positions[triangle.vertices.z]);
    // }
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData),
                 vertices.data(), GL_STATIC_DRAW);

    // Set up element array buffer (indices)
    // std::vector<glm::ivec3> indices;  // NOTE: must be unsigned int!
    std::vector<unsigned int> indices;
    for (auto& triangle : mesh.triangles) {
        indices.push_back(triangle.vertices.x);
        indices.push_back(triangle.vertices.y);
        indices.push_back(triangle.vertices.z);
        // indices.push_back(triangle.vertices);
    }

    GLuint ebo;
    glGenBuffers(1, &ebo);
    bindElementBuffer(ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // TODO: do i also assign to attributes now? Make sure after shaders
    // compiled position
    GLuint pos = glGetAttribLocation(curr_state.boundProgram, "pos");
    if (pos == -1) {
        fprintf(stderr, "ERROR: pos not found or optimized out\n");
        // return false;
    }
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                          (GLvoid*)0);
    // For without EBO
    // glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    // normal
    GLuint norm = glGetAttribLocation(
        curr_state.boundProgram,
        "norm");  // Note: optimized out by linker if not used
    if (norm == -1) {
        fprintf(stderr, "ERROR: norm not found or optimized out\n");
        // return false;
    }
    glEnableVertexAttribArray(norm);
    glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                          (GLvoid*)sizeof(glm::vec3));

    // texcoords
    GLuint texcoord = glGetAttribLocation(curr_state.boundProgram, "texcoord");
    if (texcoord == -1) {
        fprintf(stderr, "ERROR: texcoord not found or optimized out\n");
        // return false;
    }
    glEnableVertexAttribArray(texcoord);  // pos
    glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),
                          (GLvoid*)(sizeof(glm::vec3) * 2));

    // TODO: unbind VAO? why? Do i want this to be self-contained? probably.
    // How do i give access to the ids tho?

    // Material
    // TODO: will upload based on grouped triangles
    upload_material(mesh.triangles[0].material);
}

void Rasterizer::upload_material(Material* material) {
    uploadVec3("material.ambient", material->K_a);
    uploadVec3("material.diffuse", material->K_d);
    uploadVec3("material.specular", material->K_s);
    uploadFloat("material.shininess", material->shininess);
    uploadFloat("material.ior", material->ior);
    uploadFloat("material.transparency", material->transparency);
    uploadVec3("material.transmission_color", material->transmission_color);

    if (material->diffuse_map_filepath) {
        uploadBool("material.has_diffuse_tex", true);
        upload_texture(material->diffuse_map_filepath.get(), "diffuse_tex", 0);
    }
    if (material->ambient_map_filepath) {
        uploadBool("material.has_ambient_tex", true);
        upload_texture(material->ambient_map_filepath.get(), "ambient_tex", 1);
    }
    if (material->specular_map_filepath) {
        uploadBool("material.has_specular_tex", true);
        upload_texture(material->specular_map_filepath.get(), "specular_tex",
                       2);
    }
    if (material->bump_map_filepath) {
        uploadBool("material.has_bump_tex", true);
        upload_texture(material->bump_map_filepath.get(), "bump_tex", 3);
    }
}

void Rasterizer::upload_texture(TextureMap* texture, const GLchar* shaderVar,
                                int textureIndex) {
    // TODO: For now, just diffuse, then abstract out
    GLuint texID;
    glGenTextures(1, &texID);  // Could move this to upload_material to generate
                               // all that i need at once
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, &texture->pixels[0]);

    // Filters: mipmap must be generated for default filter!
    glGenerateMipmap(GL_TEXTURE_2D);

    // Tiling

    GLint sampler = glGetUniformLocation(curr_state.boundProgram, shaderVar);
    if (sampler == -1) {
        fprintf(stderr,
                "ERROR: %s sampler uniform not found or optimized out\n",
                shaderVar);
    }
    glUseProgram(curr_state.boundProgram);  // Is this needed?
    glUniform1i(sampler, textureIndex);
}

void Rasterizer::uploadVec3(const GLchar* varName, glm::vec3 data) {
    auto location = glGetUniformLocation(curr_state.boundProgram, varName);
    if (location == -1) {
        fprintf(stderr, "ERROR: %s uniform not found or optimized out\n",
                varName);
    }
    glUniform3fv(location, 1, &data[0]);
}

void Rasterizer::uploadFloat(const GLchar* varName, float data) {
    auto location = glGetUniformLocation(curr_state.boundProgram, varName);
    if (location == -1) {
        fprintf(stderr, "ERROR: %s uniform not found or optimized out\n",
                varName);
    }
    glUniform1f(location, data);
}

void Rasterizer::uploadBool(const GLchar* varName, bool data) {
    auto location = glGetUniformLocation(curr_state.boundProgram, varName);
    if (location == -1) {
        fprintf(stderr, "ERROR: %s uniform not found or optimized out\n",
                varName);
    }
    glUniform1i(location, data);
}