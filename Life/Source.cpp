#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "basewin.h"
#include <vector>

struct Rect {
	int x1;
	int y1;
	int x2;
	int y2;
};

struct Cell {
	int x;
	int y;
	bool alive;
	bool userMade;
};

class MainWindow : public BaseWindow<MainWindow> {
private:
	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
	float previousX = 0; //For right-click purposes.
	float previousY = 0;
	bool paused = true;
	bool ctrlDown = false;
	unsigned int speed = 0;
	unsigned int renderSpeed = 2;
	bool panning = false;
	bool canZoom = true;

	D2D1_POINT_2F focus; //Represents the coordinates of the upper-left corner of the screen. Keeps track of where the camera is.
	BYTE zoom; //Keeps track of how zoomed in the picture is. 1 zoom = each cell is 1 pixel, 2 zoom = each cell is 2 pixels, etc.
	std::vector<Cell> aliveCells; //A dynamic array of all currently alive cells.

	ID2D1Factory* pFactory = 0;
	ID2D1HwndRenderTarget* pRenderTarget = 0;
	ID2D1SolidColorBrush* pBrush = 0;

public:

	MainWindow() { 
		focus.x = 0; focus.y = 0; zoom = 10; 
	}

	void RenderScene() { //Draws the whole scene.
		pRenderTarget->BeginDraw();
		D2D1_RECT_F rectangle = D2D1::RectF(0, 0, 10000, 10000);
		pBrush->SetColor(D2D1::ColorF(0.2f, 0.2f, 0.2f));
		pRenderTarget->FillRectangle(&rectangle, pBrush);
		pBrush->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));
		for (int i = 0; i < aliveCells.size(); i++) {
			D2D1_RECT_F rectangle = D2D1::RectF((aliveCells[i].x) * zoom + focus.x, (aliveCells[i].y) * zoom + focus.y, (aliveCells[i].x) * zoom + zoom + focus.x, (aliveCells[i].y) * zoom + zoom + focus.y);
			pRenderTarget->FillRectangle(&rectangle, pBrush);
		}
		pBrush->SetColor(D2D1::ColorF(0.3f, 0.3f, 0.3f));
		for (int i = 0; i < 1920; i++) {
			pRenderTarget->DrawLine(D2D1::Point2F(i * zoom + (int)focus.x % zoom, 0), D2D1::Point2F(i * zoom + (int)focus.x % zoom, 1080), pBrush, 0.5, NULL);
		}
		for (int i = 0; i < 1080; i++) {
			pRenderTarget->DrawLine(D2D1::Point2F(0, i * zoom + (int)focus.y % zoom), D2D1::Point2F(1920 * 2, i * zoom + (int)focus.y % zoom), pBrush, 0.5, NULL);
		}
		if (paused == true) {
			D2D1_RECT_F rectangle = D2D1::RectF(5, 5, 25, 25);
			pBrush->SetColor(D2D1::ColorF(0.7f, 0.0f, 0.0f));
			pRenderTarget->FillRectangle(&rectangle, pBrush);
		}
		for (int i = 0; i < 7 - renderSpeed; i++) {
			D2D1_RECT_F rectangle = D2D1::RectF(30 + (i*5), 5, 30 + (i*5) + 5, 25);
			pBrush->SetColor(D2D1::ColorF((i * 42) / 255.0, 1.0 - (i * 42) / 255.0, 0.0));
			pRenderTarget->FillRectangle(&rectangle, pBrush);
		}
		pRenderTarget->EndDraw();
	}

	Rect CalculateRectangle() { //Calculates the smallest rectangle around aliveCells and returns the rectangle. Used for making a grid of cells.
		int smallestX = INT_MAX;
		int smallestY = INT_MAX;
		int biggestX = INT_MIN;
		int biggestY = INT_MIN;
		for (int i = 0; i < aliveCells.size(); i++) {
			if (aliveCells[i].x < smallestX) {
				smallestX = aliveCells[i].x;
			}
			if (aliveCells[i].x > biggestX) {
				biggestX = aliveCells[i].x;
			}
			if (aliveCells[i].y < smallestY) {
				smallestY = aliveCells[i].y;
			}
			if (aliveCells[i].y > biggestY) {
				biggestY = aliveCells[i].y;
			}
		}
		Rect rect;
		rect.x1 = smallestX - 2;
		rect.y1 = smallestY - 2;
		rect.x2 = biggestX + 3;
		rect.y2 = biggestY + 3;
		return rect;
	}

	PCWSTR  ClassName() const { return L"Sample Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_TIMER: {
		speed++;
		if (speed % (int)pow(2, renderSpeed) != 0) {
			if (panning == true) {
				RenderScene();
			}
			return 0;
		}
		Rect rectangleInt = CalculateRectangle();
		int const gridWidth = rectangleInt.x2 - rectangleInt.x1;
		int const gridHeight = rectangleInt.y2 - rectangleInt.y1;
		Cell** grid = new Cell * [gridHeight];
		for (int i = 0; i < gridHeight; i++) {
			grid[i] = new Cell[gridWidth];
			for (int j = 0; j < gridWidth; j++) {
				grid[i][j].x = j + rectangleInt.x1;
				grid[i][j].y = i + rectangleInt.y1;
				grid[i][j].alive = false;
				grid[i][j].userMade = false;
			}
		}

		for (int i = 0; i < aliveCells.size(); i++) {
			grid[aliveCells[i].y - rectangleInt.y1][aliveCells[i].x - rectangleInt.x1].alive = true;
		}

		aliveCells.clear();
		for (int i = 1; i < gridHeight - 1; i++) {
			for (int j = 1; j < gridWidth - 1; j++) {
				int neighborsAlive = 0;
				if (grid[i - 1][j].alive == true) {
					neighborsAlive++;
				}
				if (grid[i - 1][j - 1].alive == true) {
					neighborsAlive++;
				}
				if (grid[i - 1][j + 1].alive == true) {
					neighborsAlive++;
				}
				if (grid[i][j - 1].alive == true) {
					neighborsAlive++;
				}
				if (grid[i][j + 1].alive == true) {
					neighborsAlive++;
				}
				if (grid[i + 1][j].alive == true) {
					neighborsAlive++;
				}
				if (grid[i + 1][j - 1].alive == true) {
					neighborsAlive++;
				}
				if (grid[i + 1][j + 1].alive == true) {
					neighborsAlive++;
				}
				if ((neighborsAlive == 2 && grid[i][j].alive == true) || neighborsAlive == 3) {
					Cell newCell;
					newCell.alive = true;
					newCell.x = grid[i][j].x;
					newCell.y = grid[i][j].y;
					newCell.userMade = false;
					aliveCells.push_back(newCell);
				}
			}
		}

		for (int i = 0; i < gridHeight; i++) {
			delete[] grid[i];
		}
		delete[] grid;

		RenderScene();
		return 0;
	}

	case WM_KEYUP: {
		if (wParam == VK_CONTROL) {
			ctrlDown = false;
		}
		return 0;
	}

	case WM_KEYDOWN: {
		if (wParam == VK_RIGHT && renderSpeed > 1) {
			speed = 1;
			renderSpeed--;
			RenderScene();
		}
		if (wParam == VK_LEFT && renderSpeed < 6) {
			speed = 1;
			renderSpeed++;
			RenderScene();
		}
		if (wParam == 0x43) {
			paused = true;
			KillTimer(m_hwnd, 101);
			aliveCells.clear();
			RenderScene();
		}
		if (wParam == VK_CONTROL) {
			ctrlDown = true;
		}
		if (wParam == 0x5A && ctrlDown == true) {
			int position = aliveCells.size() - 1;
			int count = 1;
			while (true) {
				if (aliveCells[position].x == aliveCells[position - 1].x && aliveCells[position].y == aliveCells[position - 1].y && aliveCells.size() > 0) {
					count++;
					position--;
				} else {
					break;
				}
			}
			for (int i = 0; i < count; i++) {
				if (aliveCells.size() > 0 && aliveCells[aliveCells.size() - 1].userMade == true) {
					aliveCells.pop_back();
				}
			}
			RenderScene();
		}

		if (wParam == VK_SPACE && paused == true) {
			paused = false;
			RenderScene();
			SetTimer(m_hwnd, 101, 15, NULL);
		} else if (wParam == VK_SPACE && paused == false) {
			paused = true;
			RenderScene();
			KillTimer(m_hwnd, 101);
		}
		return 0;
	}

	case WM_LBUTTONDOWN: {
		if (paused == false) {
			return 0;
		}
		float x = LOWORD(lParam);
		float y = HIWORD(lParam);
		Cell newCell;
		float convertX = (x - focus.x) / zoom;
		float convertY = (y - focus.y) / zoom;
		if (convertX > 0) {
			newCell.x = (int)convertX;
		} else if (convertX < 0) {
			newCell.x = (int)(convertX)-1;
		} else {
			newCell.x = 0;
		}
		if (convertY > 0) {
			newCell.y = (int)convertY;
		} else if (convertY < 0) {
			newCell.y = (int)(convertY)-1;
		} else {
			newCell.y = 0;
		}
		newCell.userMade = true;
		newCell.alive = true;
		aliveCells.push_back(newCell);
		RenderScene();
		return 0;
	}

	case WM_RBUTTONUP: {
		panning = false;
	}

	case WM_MOUSEMOVE: {
		SetCursor(hCursor);

		float x = LOWORD(lParam);
		float y = HIWORD(lParam);
		if (wParam == MK_RBUTTON) { //Pans the image around.
			panning = true;
			focus.x += x - previousX;
			focus.y += y - previousY;
			if (paused == true) {
				RenderScene();
			}
		}
		if (wParam == MK_LBUTTON) { //Draws cells at the start.
			if (paused == false) {
				previousX = x;
				previousY = y;
				return 0;
			}
			Cell newCell;
			float convertX = (x - focus.x) / zoom;
			float convertY = (y - focus.y) / zoom;
			if (convertX > 0) {
				newCell.x = (int)convertX;
			} else if (convertX < 0) {
				newCell.x = (int)(convertX) - 1;
			} else {
				newCell.x = 0;
			}
			if (convertY > 0) {
				newCell.y = (int)convertY;
			} else if (convertY < 0) {
				newCell.y = (int)(convertY)-1;
			} else {
				newCell.y = 0;
			}
			newCell.userMade = true;
			newCell.alive = true;
			aliveCells.push_back(newCell);
			RenderScene();
		}
		previousX = x;
		previousY = y;
		return 0;
	}

	case WM_MOUSEWHEEL: { //Zooms in and out.
		if (canZoom == false) {
			return 0;
		}
		GET_WHEEL_DELTA_WPARAM(wParam);
		if ((int)wParam > 0 && zoom < 200) {
			focus.x -= (int)((previousX - focus.x) / zoom);
			focus.y -= (int)((previousY - focus.y) / zoom);
			zoom++;
		} else if ((int)wParam < 0 && zoom != 2) {
			focus.x += (int)((previousX - focus.x) / zoom);
			focus.y += (int)((previousY - focus.y) / zoom);
			zoom--;
		}
		RenderScene();
		return 0;
	}

	case WM_SIZE: {
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
		pRenderTarget->Resize(size);
		RenderScene();
		return 0;
	}

	case WM_CREATE: {
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) {
			return -1;
		}
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
		pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);
		pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &pBrush);

		HMENU hMenuBar = CreateMenu();
		HMENU hFile = CreateMenu();
		HMENU hOptions = CreateMenu();
		AppendMenu(hMenuBar, MF_POPUP, 0, L"About");
		SetMenu(m_hwnd, hMenuBar);
		return 0;
	}

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

	case WM_COMMAND: {
		canZoom = false;
		if (MessageBox(m_hwnd, L"This program is a simulation of Conway's Game of Life. It is played on an infinite grid of cells that can be dead or alive. Every cell interacts with its 8 neighboring cells. At each step in time, the following rules are applied:\n -Alive cells with fewer than 2 alive neighbors die, as if by underpopulation.\n -Alive cells with 2 or 3 alive neighbors will live onto the next generation.\n -Alive cells with more than 3 alive neighbors will die, as if by overpopulation.\n -Dead cells with 3 alive neighbors will become alive, as if by reproduction.\n\nInstructions:\n -Right click and drag to move the grid around.\n -Use the mousewheel to zoom in and out.\n -Left click and drag while the simulation is paused to create alive cells.\n -Press CTRL+Z to undo the last mouse click.\n -Press SPACEBAR to pause/unpause the simulation.\n -Press C to clear all currently alive cells.\n -Press the right and left arrow keys to speed up and slow down the simulation.", L"About LIFE", MB_OK) == IDOK) {
			canZoom = true;
		}
		return 0;
	}

	default: {
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	}
	return TRUE;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
	MainWindow win;

	if (!win.Create(hInstance, L"LIFE", WS_TILEDWINDOW)) {
		return 0;
	}
	ShowWindow(win.Window(), nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}