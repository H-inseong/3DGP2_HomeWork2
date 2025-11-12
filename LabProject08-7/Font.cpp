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
				if (sv_token.rfind("base", 0) == 0) m_nLineHeight = std::stoi(std::string(sv_token.substr(5)));
				if (sv_token.rfind("sclaeW", 0) == 0) m_nLineHeight = std::stoi(std::string(sv_token.substr(7)));
				if (sv_token.rfind("sclaeH", 0) == 0) m_nLineHeight = std::stoi(std::string(sv_token.substr(7)));
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
		file.close();
		return true;
	}



	return false;
}

void CSpriteFont::DrawString(ID3D12GraphicsCommandList* pd3dCommandList, std::string_view text, XMFLOAT2 position, XMFLOAT4 color, float fScale)
{
}
