#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>	

enum OBJ_TYPE
{
	OBJ_POINTS,
	OBJ_TRIANGLES
};

enum RENDER_TYPE
{
	RENDER_POINTS,
	RENDER_LINES,
	RENDER_TRIANGLES
};

class Object
{

public:
	struct Vertex {
		// Position
		glm::vec3 Position;
		// Normal
		glm::vec3 Normal;
		// TexCoords
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 BiTangent;
	};

	struct Vertex_Index {
		int pos_idx;
		int normal_idx;
		int texcoord_idx;
	};

	struct Face_Index {
		Vertex_Index vertex[3];
	};

	// veo and vao vector
	std::vector<Vertex> vao_vertices;
	std::vector<unsigned int> veo_indices;

	// obj original data vector
	std::vector<glm::vec3> ori_positions;
	std::vector<glm::vec3> ori_normals;
	std::vector<glm::vec2> ori_texcoords;

	// obj face index vector
	std::vector<Face_Index> indexed_faces;

	glm::vec3 obj_center;

	glm::vec4 obj_color = glm::vec4(0.0, 0.0, 0.0, 1.0);
	GLfloat shininess = 32.0f;

	std::string m_obj_path;
	std::string obj_name;

	GLuint vao, vbo;

	OBJ_TYPE m_obj_type;
	RENDER_TYPE m_render_type;

public:
	void load_obj(std::string obj_path)
	{
		int path_str_length = obj_path.size();
		std::string suffix = obj_path.substr(path_str_length - 3, path_str_length);

		if (suffix == "obj") {
			this->vao_vertices.clear();
			this->veo_indices.clear();
			this->indexed_faces.clear();

			this->ori_positions.clear();
			this->ori_normals.clear();
			this->ori_texcoords.clear();

			std::ifstream ifs;
			// Store original data vector
			try {
				ifs.open(obj_path);
				std::string one_line;
				while (getline(ifs, one_line)) {
					std::stringstream ss(one_line);
					std::string type;
					ss >> type;
					if (type == "v") {
						glm::vec3 vert_pos;
						ss >> vert_pos[0] >> vert_pos[1] >> vert_pos[2];
						this->ori_positions.push_back(vert_pos);
					}
					else if (type == "vt") {
						glm::vec2 tex_coord;
						ss >> tex_coord[0] >> tex_coord[1];
						this->ori_texcoords.push_back(tex_coord);
					}
					else if (type == "vn") {
						glm::vec3 vert_norm;
						ss >> vert_norm[0] >> vert_norm[1] >> vert_norm[2];
						this->ori_normals.push_back(vert_norm);
					}
					else if (type == "f") {
						Face_Index face_idx;
						//Vertex vertces[3];
						// Here we only accept face number 3
						Vertex vertex[3];
						for (int i = 0; i < 3; i++) {
							std::string s_vertex;
							ss >> s_vertex;
							int pos_idx = -1;
							int tex_idx = -1;
							int norm_idx = -1;
							sscanf(s_vertex.c_str(), "%d/%d/%d", &pos_idx, &tex_idx, &norm_idx);
							// We have to use index -1 because the obj index starts at 1
							// Incorrect input will be set as -1
							face_idx.vertex[i].pos_idx = pos_idx > 0 ? pos_idx - 1 : -1;
							face_idx.vertex[i].texcoord_idx = tex_idx > 0 ? tex_idx - 1 : -1;
							face_idx.vertex[i].normal_idx = norm_idx > 0 ? norm_idx - 1 : -1;
							vertex[i].Position = ori_positions[pos_idx - 1];
							vertex[i].TexCoords = ori_texcoords[tex_idx - 1];
							vertex[i].TexCoords.y *= -1;
							vertex[i].Normal = ori_normals[norm_idx - 1];
						}
						indexed_faces.push_back(face_idx);

						glm::vec2 UV1 = vertex[1].TexCoords - vertex[0].TexCoords;
						glm::vec2 UV2 = vertex[2].TexCoords - vertex[0].TexCoords;
						glm::vec3 V1 = vertex[1].Position - vertex[0].Position;
						glm::vec3 V2 = vertex[2].Position - vertex[0].Position;
						float d = 1.0f / (UV1.x * UV2.y - UV1.y * UV2.x);
						glm::vec3 tangent;// = (V1 * UV2.y - V2 * UV1.y)*d;
						tangent.x = d * (UV2.y * V1.x - UV1.y * V2.x);
						tangent.y = d * (UV2.y * V1.y - UV1.y * V2.y);
						tangent.z = d * (UV2.y * V1.z - UV1.y * V2.z);
						tangent = glm::normalize(tangent);

						glm::vec3 bitangent;// = (V2 * UV1.x - V1 * UV2.x)*d;
						bitangent.x = d * (-UV2.x * V1.x + UV1.x * V2.x);
						bitangent.y = d * (-UV2.x * V1.y + UV1.x * V2.y);
						bitangent.z = d * (-UV2.x * V1.z + UV1.x * V2.z);
						bitangent = glm::normalize(bitangent);

						vertex[0].Tangent = tangent;
						vertex[1].Tangent = tangent;
						vertex[2].Tangent = tangent;
						vertex[0].BiTangent = bitangent;
						vertex[1].BiTangent = bitangent;
						vertex[2].BiTangent = bitangent;

						vao_vertices.push_back(vertex[0]);
						vao_vertices.push_back(vertex[1]);
						vao_vertices.push_back(vertex[2]);
					}
				}
			}
			catch (const std::exception&) {
				std::cout << "Error: Obj file cannot be read\n";
			}

			/*
			// Retrieve data from index and assign to vao and veo
			for (int i = 0; i < indexed_faces.size(); i++) {
				Face_Index cur_idx_face = indexed_faces[i];
				// If no normal: recalculate for them
				glm::vec3 v0 = ori_positions[cur_idx_face.vertex[0].pos_idx];
				glm::vec3 v1 = ori_positions[cur_idx_face.vertex[1].pos_idx];
				glm::vec3 v2 = ori_positions[cur_idx_face.vertex[2].pos_idx];
				glm::vec3 new_normal = glm::cross(v1 - v0, v2 - v0);

				Vertex cur_vertex[3];
				for (int j = 0; j < 3; j++) {
					//Vertex cur_vertex;
					Vertex_Index cur_idx_vertex = cur_idx_face.vertex[j];
					if (cur_idx_vertex.pos_idx >= 0) {
						cur_vertex[j].Position = ori_positions[cur_idx_vertex.pos_idx];
					}
					if (cur_idx_vertex.normal_idx >= 0) {
						cur_vertex[j].Normal = ori_normals[cur_idx_vertex.normal_idx];
					}
					else {
						cur_vertex[j].Normal = new_normal;
					}
					if (cur_idx_vertex.texcoord_idx >= 0) {
						cur_vertex[j].TexCoords = ori_texcoords[cur_idx_vertex.texcoord_idx];
						cur_vertex[j].TexCoords.y *= -1;
						
					}
					
				}
				glm::vec2 UV1, UV2;
				glm::vec3 V1, V2;
				float d;
				glm::vec3 tangent;
				glm::vec3 bitangent;
				for (int j = 0; j < 3; j++)
				{
					UV1 = cur_vertex[(j + 1) % 3].TexCoords - cur_vertex[j % 3].TexCoords;
					UV2 = cur_vertex[(j + 2) % 3].TexCoords - cur_vertex[j % 3].TexCoords;
					V1 = cur_vertex[(j + 1) % 3].Position - cur_vertex[j % 3].Position;
					V2 = cur_vertex[(j + 2) % 3].Position - cur_vertex[j % 3].Position;
					d = 1.0f / (UV1.x * UV2.y - UV1.y * UV2.x);
					tangent.x = d * (UV2.y * V1.x - UV1.y * V2.x);
					tangent.y = d * (UV2.y * V1.y - UV1.y * V2.y);
					tangent.z = d * (UV2.y * V1.z - UV1.y * V2.z);
					tangent = glm::normalize(tangent);
					bitangent.x = d * (-1 * UV2.x * V1.x + UV1.x * V2.x);
					bitangent.y = d * (-1 * UV2.x * V1.y + UV1.x * V2.y);
					bitangent.z = d * (-1 * UV2.x * V1.z + UV1.x * V2.z);
					bitangent = glm::normalize(bitangent);
					cur_vertex[j].Tangent = tangent;
					cur_vertex[j].BiTangent = bitangent;
					vao_vertices.push_back(cur_vertex[j]);
					veo_indices.push_back(i * 3 + j);
				}
			}
			*/
		}
	};

	void calculate_center()
	{
		glm::vec3 max_bound(INT_MIN);
		glm::vec3 min_bound(INT_MAX);
		for (auto vertex : this->vao_vertices) {
			max_bound[0] = std::max(vertex.Position[0], max_bound[0]);
			max_bound[1] = std::max(vertex.Position[1], max_bound[1]);
			max_bound[2] = std::max(vertex.Position[2], max_bound[2]);
			min_bound[0] = std::min(vertex.Position[0], min_bound[0]);
			min_bound[1] = std::min(vertex.Position[1], min_bound[1]);
			min_bound[2] = std::min(vertex.Position[2], min_bound[2]);
		}
		this->obj_center = (max_bound + min_bound) * 0.5f;

	};

	// Initialize object using obj file
	Object(std::string obj_path) {
		this->m_obj_path = obj_path;
		load_obj(this->m_obj_path);
		calculate_center();
		m_obj_type = OBJ_TRIANGLES;
		m_render_type = RENDER_TRIANGLES;
	};

	// Initialize object using point array, can used to draw 
	Object(std::vector<glm::vec3> points)
	{
		this->vao_vertices.clear();
		this->veo_indices.clear();
		this->indexed_faces.clear();

		this->ori_positions.clear();
		this->ori_normals.clear();
		this->ori_texcoords.clear();
		for (unsigned int i = 0; i < points.size(); i++)
		{
			glm::vec3 cur_point = points[i];
			this->ori_positions.push_back(cur_point);
			Vertex cur_vertex;
			cur_vertex.Position = cur_point;
			this->vao_vertices.push_back(cur_vertex);
		}
		calculate_center();
		m_obj_type = OBJ_POINTS;
		m_render_type = RENDER_POINTS;
	};

	~Object() {};
};