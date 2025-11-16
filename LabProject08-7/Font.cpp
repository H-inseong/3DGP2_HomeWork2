#include "stdafx.h"
#include "Font.h"
#include "d3dx12.h"

#include <fstream>
#include <sstream> 



CSpriteFont::CSpriteFont(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CTexture* pTexture, int MAX_CHARS, ID3D12DescriptorHeap* pd3dCbvSrvUavDescriptorHeap, UINT nCbvSrvUavDescriptorIncrementSize)
	: m_pd3dDevice(pd3dDevice)
	, m_pd3dCommandList(pd3dCommandList)
	, m_pTexture(pTexture)
	, m_MAX_CHARS(MAX_CHARS)
	, m_pd3dCbvSrvUavDescriptorHeap(pd3dCbvSrvUavDescriptorHeap)
	, m_nCbvSrvUavDescriptorIncrementSize(nCbvSrvUavDescriptorIncrementSize)
	, m_vCharInfos(static_cast<size_t>(MAX_CHARS))
{
	if (m_pTexture) { m_pTexture->AddRef(); }
	m_vCharInfos.resize(MAX_CHARS);

	D3D12_RESOURCE_DESC d3dResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(CB_FONT_INFO) * m_MAX_CHARS);
	D3D12_HEAP_PROPERTIES d3dHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	HRESULT hResult = m_pd3dDevice->CreateCommittedResource(
		&d3dHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pd3dcbFontInfo));
	
	m_pd3dcbFontInfo->Map(0, nullptr, reinterpret_cast<void**>(&m_pcbMappedFontInfo));

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
	SrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Buffer.FirstElement = 0;
	SrvDesc.Buffer.NumElements = MAX_CHARS;
	SrvDesc.Buffer.StructureByteStride = sizeof(CB_FONT_INFO);
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	
	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	SrvCpuHandle.ptr += m_nCbvSrvUavDescriptorIncrementSize * 1;

	m_pd3dDevice->CreateShaderResourceView(m_pd3dcbFontInfo, &SrvDesc, SrvCpuHandle);
	
	m_d3dFontSrvGpuDescriptorHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dFontSrvGpuDescriptorHandle.ptr += m_nCbvSrvUavDescriptorIncrementSize * 1;

	m_pd3dFontVertexBuffer = CreateBufferResource(
		m_pd3dDevice,
		m_pd3dCommandList,
		nullptr,
		sizeof(FONT_VERTEX) * MAX_CHARS,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr);
	m_pd3dFontVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pMappedFontVertices));

	m_d3dFontVertexBufferView.BufferLocation = m_pd3dFontVertexBuffer->GetGPUVirtualAddress();
	m_d3dFontVertexBufferView.StrideInBytes = sizeof(FONT_VERTEX);
	m_d3dFontVertexBufferView.SizeInBytes = sizeof(FONT_VERTEX) * MAX_CHARS;
}

CSpriteFont::~CSpriteFont()
{
	if(m_pTexture) { m_pTexture->Release();	}

	if(m_pd3dcbFontInfo && m_pcbMappedFontInfo)
	{
		m_pd3dcbFontInfo->Unmap(0, nullptr);
		m_pcbMappedFontInfo = nullptr;
	}
	if (m_pd3dcbFontInfo)
	{
		m_pd3dcbFontInfo->Unmap(0, nullptr);
		m_pd3dcbFontInfo->Release();
	}
	if (m_pd3dFontVertexBuffer)
	{
		m_pd3dFontVertexBuffer->Unmap(0, nullptr);
		m_pd3dFontVertexBuffer->Release();
	}

}

bool CSpriteFont::LoadFontData(std::string_view filename)
{
	std::ifstream file{std::string(filename)};
	
	if (!file.is_open()) {
		return false;
	}

	std::vector<CharInfo> tempCharInfos(static_cast<size_t>(m_MAX_CHARS));

	std::string line;
	while(std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string type;
		ss >> type;
		if (type == "common")
		{
			std::string token;
			while (ss >> token)
			{
				std::string_view sv_token(token);
				if (sv_token.rfind("lineHeight", 0) == 0) m_nLineHeight = std::stoi(std::string(sv_token.substr(11)));
				else if (sv_token.rfind("base", 0) == 0) m_nBase = std::stoi(std::string(sv_token.substr(5)));
				else if (sv_token.rfind("scaleW", 0) == 0) m_nScaleW = std::stoi(std::string(sv_token.substr(7)));
				else if (sv_token.rfind("scaleH", 0) == 0) m_nScaleH = std::stoi(std::string(sv_token.substr(7)));
			}
		}
		else if (type == "char")
		{
			CharInfo charInfo{};
			std::string token;
			bool id_found = false;

			while (ss >> token)
			{
				std::string_view sv_token(token);
				if (sv_token.rfind("id=", 0) == 0) {
					charInfo.id = std::stoi(std::string(sv_token.substr(3)));
					id_found = true;
				}
				else if (sv_token.rfind("x=", 0) == 0)		charInfo.x = std::stoi(std::string(sv_token.substr(2)));
				else if (sv_token.rfind("y=", 0) == 0)		charInfo.y = std::stoi(std::string(sv_token.substr(2)));
				else if (sv_token.rfind("width=", 0) == 0)	charInfo.width = std::stoi(std::string(sv_token.substr(6)));
				else if (sv_token.rfind("height=", 0) == 0) charInfo.height = std::stoi(std::string(sv_token.substr(7)));
				else if (sv_token.rfind("xoffset=", 0) == 0) charInfo.xoffset = std::stoi(std::string(sv_token.substr(8)));
				else if (sv_token.rfind("yoffset=", 0) == 0) charInfo.yoffset = std::stoi(std::string(sv_token.substr(8)));
				else if (sv_token.rfind("xadvance=", 0) == 0) charInfo.xadvance = std::stoi(std::string(sv_token.substr(9)));
				else if (sv_token.rfind("page=", 0) == 0)	charInfo.page = std::stoi(std::string(sv_token.substr(5)));
				else if (sv_token.rfind("chnl=", 0) == 0)	charInfo.chnl = std::stoi(std::string(sv_token.substr(5)));
			}

			if(id_found && charInfo.id >= 0 && charInfo.id < static_cast<int>(m_vCharInfos.size()))
			{
				tempCharInfos[charInfo.id] = charInfo;
			}
			else
			{
				//ERROR
				return false;
			}
		}
	}
	file.close();

	memcpy(m_pcbMappedFontInfo, tempCharInfos.data(), sizeof(CharInfo)* m_MAX_CHARS);
	m_vCharInfos = std::move(tempCharInfos);

	return true;
}

void CSpriteFont::DrawString(ID3D12GraphicsCommandList* pd3dCommandList, std::string_view text, XMFLOAT2 position, XMFLOAT4 color, float fScale)
{
	//if (!m_pTexture) return;
	//float x = position.x;
	//float y = position.y;
	//pd3dCommandList->SetPipelineState(m_pTexture->GetPipelineState());
	//pd3dCommandList->SetGraphicsRootSignature(m_pTexture->GetRootSignature());
	//m_pTexture->SetTexture(pd3dCommandList, 0);
	//for (char c : text)
	//{
	//	if (c < 0 || c >= static_cast<char>(m_vCharInfos.size())) continue;
	//	CharInfo& charInfo = m_vCharInfos[static_cast<size_t>(c)];
	//	float u0 = static_cast<float>(charInfo.x) / static_cast<float>(m_nScaleW);
	//	float v0 = static_cast<float>(charInfo.y) / static_cast<float>(m_nScaleH);
	//	float u1 = static_cast<float>(charInfo.x + charInfo.width) / static_cast<float>(m_nScaleW);
	//	float v1 = static_cast<float>(charInfo.y + charInfo.height) / static_cast<float>(m_nScaleH);
	//	float w = charInfo.width * fScale;
	//	float h = charInfo.height * fScale;
	//	// Draw character quad here using the calculated UVs and dimensions
	//	// This part is dependent on your rendering setup and is omitted for brevity
	//	x += charInfo.xadvance * fScale;
	//}
}