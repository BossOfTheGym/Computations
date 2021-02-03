#pragma once

#include <glfw-cxx/glfw3.h>
#include <entt/signal/delegate.hpp>
#include <entt/signal/sigh.hpp>

namespace win
{
	class MainWindow : public glfw::Window
	{
	public:
		using KeyPressSigh          = entt::sigh<void(int, int, int, int)>;
		using CharSigh              = entt::sigh<void(unsigned int)>;
		using MouseMoveSigh         = entt::sigh<void(double, double)>;
		using MouseButtonSigh       = entt::sigh<void(int, int, int)>;
		using MouseEnterSigh        = entt::sigh<void(int)>;
		using ScrollSigh            = entt::sigh<void(double, double)>;
		using DropSigh              = entt::sigh<void(int, const char*[])>;
		using CloseSigh             = entt::sigh<void()>;
		using ResizeSigh            = entt::sigh<void(int, int)>;
		using FramebufferResizeSigh = entt::sigh<void(int, int)>;
		using MoveSigh              = entt::sigh<void(int, int)>;
		using MinimizeSigh          = entt::sigh<void(int)>;
		using MaximizeSigh          = entt::sigh<void(int)>;
		using FocusSigh             = entt::sigh<void(int)>;
		using RefreshSigh           = entt::sigh<void()>;
		using ContentScaleSigh      = entt::sigh<void(float, float)>;

		using KeyPressSink          = KeyPressSigh          ::sink_type;
		using CharSink              = CharSigh              ::sink_type;
		using MouseMoveSink         = MouseMoveSigh         ::sink_type;
		using MouseButtonSink       = MouseButtonSigh       ::sink_type;
		using MouseEnterSink        = MouseEnterSigh        ::sink_type;
		using ScrollSink            = ScrollSigh            ::sink_type;
		using DropSink              = DropSigh              ::sink_type;
		using CloseSink             = CloseSigh             ::sink_type;
		using ResizeSink            = ResizeSigh            ::sink_type;
		using FramebufferResizeSink = FramebufferResizeSigh ::sink_type;
		using MoveSink              = MoveSigh              ::sink_type;
		using MinimizeSink          = MinimizeSigh          ::sink_type;
		using MaximizeSink          = MaximizeSigh          ::sink_type;
		using FocusSink             = FocusSigh             ::sink_type;
		using RefreshSink           = RefreshSigh           ::sink_type;
		using ContentScaleSink      = ContentScaleSigh      ::sink_type;


	public:
		MainWindow(const glfw::CreationInfo& info = {});


	public: // sink methods, all return objects that are used to connect handlers
		KeyPressSink keyPressSink();

		CharSink charSink();

		MouseMoveSink mouseMoveSink();

		MouseButtonSink mouseButtonSink();

		MouseEnterSink mouseEnterSink();

		ScrollSink scrollSink();

		DropSink dropSink();

		CloseSink closeSink();

		ResizeSink resizeSink();

		FramebufferResizeSink framebufferResizeSink();

		MoveSink moveSink();

		MinimizeSink minimizeSink();

		MaximizeSink maximizeSink();

		FocusSink focusSink();

		RefreshSink refreshSink();

		ContentScaleSink contentScaleSink();


	private: // redirects events to signals 
		virtual void keyPressEvent(int key, int scancode, int action, int mods) override;

		virtual void charEvent(unsigned int codepoint) override;

		virtual void mouseMoveEvent(double xpos, double ypos) override;

		virtual void mouseButtonEvent(int button, int action, int mods) override;

		virtual void mouseEnterEvent(int entered) override;

		virtual void scrollEvent(double xOffset, double yOffset) override;

		virtual void dropEvent(int pathCount, const char* paths[]) override;


		virtual void closeEvent() override;

		virtual void resizeEvent(int width, int height) override;

		virtual void framebufferResizeEvent(int width, int height) override;

		virtual void moveEvent(int xPos, int yPos) override;

		virtual void minimizeEvent(int minimized) override;

		virtual void maximizeEvent(int maximized) override;

		virtual void focusEvent(int focused) override;

		virtual void refreshEvent() override;

		virtual void contentScaleEvent(float xScale, float yScale) override;


	private:
		KeyPressSigh          m_keyPressSigh;
		CharSigh              m_charSigh;
		MouseMoveSigh         m_mouseMoveSigh;
		MouseButtonSigh       m_mouseButtonSigh;
		MouseEnterSigh        m_mouseEnterSigh;
		ScrollSigh            m_scrollSigh;
		DropSigh              m_dropSigh;
		CloseSigh             m_closeSigh;
		ResizeSigh            m_resizeSigh;
		FramebufferResizeSigh m_framebufferResizeSigh;
		MoveSigh              m_moveSigh;
		MinimizeSigh          m_minimizeSigh;
		MaximizeSigh          m_maximizeSigh;
		FocusSigh             m_focusSigh;
		RefreshSigh           m_refreshSigh;
		ContentScaleSigh      m_contentScaleSigh;
	};	
}
