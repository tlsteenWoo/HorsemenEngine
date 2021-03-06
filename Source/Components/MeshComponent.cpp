#include "HorsemanStd.h"
#include "MeshComponent.h"

#include "Camera.h"
#include "TransformComponent.h"
#include "Input/Input.h"
#include "Rendering/Texture.h"
#include "Utils/loaders.h"

const char* MeshComponent::g_Name = "MeshComponent";

MeshComponent::MeshComponent() {
}

MeshComponent::~MeshComponent() {
}

bool MeshComponent::VInit(rapidxml::xml_node<>* pNode) {
	m_pTexture = new Texture();

	try {
		const char* pTexture = pNode->first_node("Texture")->first_attribute("src")->value();
		m_TexturePath = new char[strlen(pTexture) + 1];
		strcpy(m_TexturePath, pTexture);

		const char* pMesh = pNode->first_node("Mesh")->first_attribute("src")->value();
		m_MeshPath = new char[strlen(pMesh) + 1];
		strcpy(m_MeshPath, pMesh);
	}
	catch (const std::runtime_error& e)
	{
		printf("[MeshComponent ERROR] Runtime error: %s\n", e.what());
		return false;
	}
	catch (const rapidxml::parse_error& e)
	{
		printf("[MeshComponent ERROR] Parse error: %s\n", e.what());
		return false;
	}
	catch (const std::exception& e)
	{
		printf("[MeshComponent ERROR] General error: %s\n", e.what());
		return false;
	}
	catch (int e) {
		printf("[MeshComponent ERROR] Unknown error occured {ErrorCode: %i}\n", e);
		return false;
	}

	return true;
}

void MeshComponent::VPostInit() {
	m_pTexture->LoadDDS(m_TexturePath);
	bool res = LoadObj(m_MeshPath, m_Vertices, m_UVs, m_Normals);
	if (!res) { printf("ModelLoadError [%s]: Model could not be loaded.\n", m_MeshPath); return; }

	// Create VAO
	glGenVertexArrays(1, &m_VertexArrayID);
	glBindVertexArray(m_VertexArrayID);

	glGenBuffers(1, &m_VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(vec3), &m_Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_UVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_UVBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_UVs.size() * sizeof(vec2), &m_UVs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_NormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_NormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(vec3), &m_Normals[0], GL_STATIC_DRAW);
}

void MeshComponent::VUpdate(float dt) {
}

void MeshComponent::VRender(map<string, GLuint> handles, Camera* cam, vec3 lightPos) {
	mat4 transform = GetTransform();

	// Use shader
	glUseProgram(handles["ProgramID"]);

	// Update and send MVP
	mat4 mvp = cam->Projection * cam->View * transform;
	mat4 view = cam->View;
	glUniformMatrix4fv(handles["MatrixID"], 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(handles["ModelMatID"], 1, GL_FALSE, &transform[0][0]);
	glUniformMatrix4fv(handles["ViewMatID"], 1, GL_FALSE, &view[0][0]);

	// Bind light
	glUniform3f(handles["LightID"], lightPos.x, lightPos.y, lightPos.z);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pTexture->Image);
	glUniform1i(handles["TextureID"], 0);

	// Bind vertex array
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Bind color array
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_UVBuffer);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	// Bind the normal array
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m_NormalBuffer);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_Vertices.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void MeshComponent::Cleanup() {
	glDeleteBuffers(1, &m_VertexBuffer);
	glDeleteBuffers(1, &m_UVBuffer);
	glDeleteVertexArrays(1, &m_VertexArrayID);

	m_pTexture->Cleanup();
	SAFE_DELETE(m_pTexture);
}

mat4 MeshComponent::GetTransform() {
	shared_ptr<TransformComponent> pTransform = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
	return pTransform->Transform;
}
