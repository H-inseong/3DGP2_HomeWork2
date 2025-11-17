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

	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = sizeof(Fnt_Data) * m_MAX_CHARS;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hResult = m_pd3dDevice->CreateCommittedResource(
		&d3dHeapPropertiesDesc,
		D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		__uuidof(ID3D12Resource),
		(void**)&m_pd3dcbFntInfo);

	
	if (m_pd3dcbFntInfo) {
		m_pd3dcbFntInfo->Map(0, nullptr, reinterpret_cast<void**>(&m_pcbMappedFntInfo));
	} else {
		// 에러 처리
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc{};
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Buffer.FirstElement = 0;
	SrvDesc.Buffer.NumElements = m_MAX_CHARS;
	SrvDesc.Buffer.StructureByteStride = sizeof(Fnt_Data);
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	
	D3D12_CPU_DESCRIPTOR_HANDLE SrvCpuHandle = m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	SrvCpuHandle.ptr += m_nCbvSrvUavDescriptorIncrementSize * 1;

	HRESULT a = pd3dDevice->GetDeviceRemovedReason();
	m_pd3dDevice->CreateShaderResourceView(m_pd3dcbFntInfo, &SrvDesc, SrvCpuHandle);
	a = pd3dDevice->GetDeviceRemovedReason();
	m_d3dFontSrvGpuDescriptorHandle = m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	m_d3dFontSrvGpuDescriptorHandle.ptr += m_nCbvSrvUavDescriptorIncrementSize * 1;

	UINT nBufferSize = sizeof(FONT_VERTEX) * m_MAX_CHARS;	// 한 문장에 최대 작성 가능한 문자 수
	m_pd3dFontVertexBuffer = ::CreateBufferResource(
		m_pd3dDevice,
		m_pd3dCommandList,
		NULL,
		nBufferSize,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL);

	m_pd3dFontVertexBuffer->Map(0, nullptr, (void**)(&m_pMappedFontVertices));

	m_d3dFontVertexBufferView.BufferLocation = m_pd3dFontVertexBuffer->GetGPUVirtualAddress();
	m_d3dFontVertexBufferView.StrideInBytes = sizeof(FONT_VERTEX);
	m_d3dFontVertexBufferView.SizeInBytes = sizeof(FONT_VERTEX) * m_MAX_CHARS;
}

CSpriteFont::~CSpriteFont()
{
	if(m_pTexture) { m_pTexture->Release();	}

	if(m_pd3dcbFntInfo && m_pcbMappedFntInfo)
	{
		m_pd3dcbFntInfo->Unmap(0, nullptr);
		m_pcbMappedFntInfo = nullptr;
	}
	if (m_pd3dcbFntInfo)
	{
		m_pd3dcbFntInfo->Unmap(0, nullptr);
		m_pd3dcbFntInfo->Release();
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

	std::vector<Fnt_Data> tempCharInfos(static_cast<size_t>(m_MAX_CHARS));

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
			Fnt_Data charInfo{};
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

	memcpy(m_pcbMappedFntInfo, tempCharInfos.data(), sizeof(Fnt_Data)* m_MAX_CHARS);
	m_vCharInfos = std::move(tempCharInfos);

	return true;
}

void CSpriteFont::DrawString(ID3D12GraphicsCommandList* pd3dCommandList, std::string_view text, XMFLOAT2 position, XMFLOAT4 color, float fScale)
{
	if (!m_pTexture) return;
	UINT nOffset = 0;

	for (char c : text)
	{
		if (c < 0 || c >= static_cast<char>(m_vCharInfos.size())) continue;
		Fnt_Data& charInfo = m_vCharInfos[static_cast<size_t>(c)];
		FONT_VERTEX* pCurrentVertex = m_pMappedFontVertices + nOffset;

		pCurrentVertex->m_xmf2Position = position;
		pCurrentVertex->m_xmf4Color = color;
		pCurrentVertex->m_nType = static_cast<UINT>(c);
		
		position.x += charInfo.xadvance * fScale;
		nOffset++;
	}

	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dFontVertexBufferView);
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dCommandList->DrawInstanced(nOffset, 1, 0, 0);

}