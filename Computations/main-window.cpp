#include "main-window.h"

namespace win
{
	MainWindow::MainWindow(const glfw::CreationInfo& info) : glfw::Window(info)
	{}

	// sinks(public)
	auto MainWindow::keyPressSink() -> KeyPressSink
	{
		return KeyPressSink{m_keyPressSigh};
	}

	auto MainWindow::charSink() -> CharSink
	{
		return CharSink{m_charSigh};
	}

	auto MainWindow::mouseMoveSink() -> MouseMoveSink
	{
		return MouseMoveSink{m_mouseMoveSigh};
	}

	auto MainWindow::mouseButtonSink() -> MouseButtonSink
	{
		return MouseButtonSink{m_mouseButtonSigh};
	}

	auto MainWindow::mouseEnterSink() -> MouseEnterSink
	{
		return MouseEnterSink{m_mouseEnterSigh};
	}

	auto MainWindow::scrollSink() -> ScrollSink
	{
		return ScrollSink{m_scrollSigh};
	}

	auto MainWindow::dropSink() -> DropSink
	{
		return DropSink{m_dropSigh};
	}

	auto MainWindow::closeSink() -> CloseSink
	{
		return CloseSink{m_closeSigh};
	}

	auto MainWindow::resizeSink() -> ResizeSink 
	{
		return ResizeSink{m_resizeSigh};
	}

	auto MainWindow::framebufferResizeSink() -> FramebufferResizeSink
	{
		return FramebufferResizeSink{m_framebufferResizeSigh};
	}

	auto MainWindow::moveSink() -> MoveSink
	{
		return MoveSink{m_moveSigh};
	}

	auto MainWindow::minimizeSink() -> MinimizeSink
	{
		return MinimizeSink{m_minimizeSigh};
	}

	auto MainWindow::maximizeSink() -> MaximizeSink
	{
		return MaximizeSink{m_maximizeSigh};
	}

	auto MainWindow::focusSink() -> FocusSink
	{
		return FocusSink{m_focusSigh};
	}

	auto MainWindow::refreshSink() -> RefreshSink
	{
		return RefreshSink{m_refreshSigh};
	}

	auto MainWindow::contentScaleSink() -> ContentScaleSink 
	{
		return ContentScaleSink{m_contentScaleSigh};
	}

	// overriden(private)
	void MainWindow::keyPressEvent(int key, int scancode, int action, int mods)
	{
		m_keyPressSigh.publish(key, scancode, action, mods);
	}

	void MainWindow::charEvent(unsigned int codepoint)
	{
		m_charSigh.publish(codepoint);
	}

	void MainWindow::mouseMoveEvent(double xpos, double ypos)
	{
		m_mouseMoveSigh.publish(xpos, ypos);
	}

	void MainWindow::mouseButtonEvent(int button, int action, int mods)
	{
		m_mouseButtonSigh.publish(button, action, mods);
	}

	void MainWindow::mouseEnterEvent(int entered)
	{
		m_mouseEnterSigh.publish(entered);
	}

	void MainWindow::scrollEvent(double xOffset, double yOffset)
	{
		m_scrollSigh.publish(xOffset, yOffset);
	}

	void MainWindow::dropEvent(int pathCount, const char* paths[])
	{
		m_dropSigh.publish(pathCount, paths);
	}

	void MainWindow::closeEvent()
	{
		m_closeSigh.publish();
	}

	void MainWindow::resizeEvent(int width, int height)
	{
		m_resizeSigh.publish(width, height);
	}

	void MainWindow::framebufferResizeEvent(int width, int height)
	{
		m_framebufferResizeSigh.publish(width, height);
	}

	void MainWindow::moveEvent(int xPos, int yPos)
	{
		m_moveSigh.publish(xPos, yPos);
	}

	void MainWindow::minimizeEvent(int minimized)
	{
		m_minimizeSigh.publish(minimized);
	}

	void MainWindow::maximizeEvent(int maximized)
	{
		m_maximizeSigh.publish(maximized);
	}

	void MainWindow::focusEvent(int focused)
	{
		m_focusSigh.publish(focused);
	}

	void MainWindow::refreshEvent()
	{
		m_refreshSigh.publish();
	}

	void MainWindow::contentScaleEvent(float xScale, float yScale)
	{
		m_contentScaleSigh.publish(xScale, yScale);
	}
}
