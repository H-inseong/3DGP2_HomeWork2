#include "stdafx.h"
#include "Font.h"

#include <fstream>
#include <sstream> 

CSpriteFont::CSpriteFont(CTexture* pTexture, int MAX_FONT_CHARS = 128) : m_pTexture(pTexture)
{
	if (m_pTexture) m_pTexture->AddRef();
	m_vCharInfos.resize(MAX_FONT_CHARS);
}

CSpriteFont::~CSpriteFont()
{
	if(m_pTexture) { m_pTexture->Release();	}
}

bool CSpriteFont::LoadFontData(std::string_view filename)
{
	std::ifstream file{std::string(filename)};
	
	if (!file.is_open()) {
		return false;
	}

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
				m_vCharInfos[charInfo.id] = charInfo;
			}
			else
			{
				//ERROR
				return false;
			}
		}
	}
	file.close();
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