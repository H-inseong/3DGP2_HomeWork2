#pragma once
#include "Object.h"

struct Fnt_Data {
	int id;
	int x, y;
	int width, height;
	int	xoffset, yoffset;
	int xadvance;
	int page;
	int chnl;
};

struct FONT_VERTEX
{
	XMFLOAT2				m_xmf2Position;
	XMFLOAT4				m_xmf4Color;
	UINT					m_nType;
};

struct FONT_INFO
{
	XMFLOAT2				ScreenSize;
	XMFLOAT2				TextureSize;
	float 					Scale;
};

struct TextInfo
{
	std::string_view		text;
	XMFLOAT2				position;
	XMFLOAT4				color;
	float					fScale;
};

class CSpriteFont {
public:
	CSpriteFont(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		CTexture* pTexture,
		int MAX_CHARS = 128,
		ID3D12DescriptorHeap* pd3dCbvSrvUavDescriptorHeap = nullptr,
		UINT nCbvSrvUavDescriptorIncrementSize = 0
		);

	~CSpriteFont();

	bool LoadFontData(std::string_view filename);

	void DrawString(
		ID3D12GraphicsCommandList* pd3dCommandList,
		std::string_view text,
		XMFLOAT2 position,
		XMFLOAT4 color,
		float fScale = 1.0f);

	XMFLOAT2 GetTextureSize() const { return XMFLOAT2((float)m_nScaleW,(float)m_nScaleH); }

private:
	CTexture*				m_pTexture = nullptr;
	std::vector<Fnt_Data>	m_vCharInfos;

	int						m_nLineHeight = 0;
	int						m_nBase = 0;
	int						m_nScaleW = 0;
	int						m_nScaleH = 0;

	UINT					m_MAX_CHARS = 128;

	ID3D12Device*			m_pd3dDevice = nullptr;
	ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;
	ID3D12DescriptorHeap*	m_pd3dCbvSrvUavDescriptorHeap = nullptr;
	UINT					m_nCbvSrvUavDescriptorIncrementSize = 0;

	ID3D12Resource*			m_pd3dcbFntInfo = nullptr;
	Fnt_Data*				m_pcbMappedFntInfo = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dFontSrvGpuDescriptorHandle;

	ID3D12Resource*				m_pd3dFontVertexBuffer = nullptr;
	FONT_VERTEX*				m_pMappedFontVertices = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dFontVertexBufferView;
};