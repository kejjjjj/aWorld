#pragma once

#include "r/gui/r_gui.hpp"

class CWorldWindow : public CGuiElement
{
public:
	CWorldWindow(const std::string& id);
	~CWorldWindow() = default;

	void* GetRender() override {
		union {
			void (CWorldWindow::* memberFunction)();
			void* functionPointer;
		} converter{};
		converter.memberFunction = &CWorldWindow::Render;
		return converter.functionPointer;
	}

	void Render() override;

private:


	size_t m_uSelectedPlayback = {};
};
