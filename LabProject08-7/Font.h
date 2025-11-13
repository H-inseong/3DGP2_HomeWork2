#pragma once
#include "Object.h"

struct CharInfo {
	int id;
	int x, y;
	int width, height;
	int	xoffset, yoffset;
	int xadvance;
	int page;
	int chnl;
};

class CSpriteFont {
public:
	CSpriteFont(CTexture* pTexture, int);
	~CSpriteFont();

	bool LoadFontData(std::string_view filename);

	void DrawString(
		ID3D12GraphicsCommandList* pd3dCommandList,
		std::string_view text,
		XMFLOAT2 position,
		XMFLOAT4 color,
		float fScale = 1.0f);

private:
	CTexture*				m_pTexture = nullptr;
	std::vector<CharInfo>	m_vCharInfos;
	int						m_nLineHeight = 0;
	int						m_nBase = 0;
	int						m_nScaleW = 0;
	int						m_nScaleH = 0;
};