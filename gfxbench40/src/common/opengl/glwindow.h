/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file glwindow.h
	GLWindow class defined here.
	GLWindow is a OpenGL class interface
	It depends on the actual project configuration. Can be EGLWindow, WGLWindow or KODEWindow.
*/
#ifndef GLWINDOW_H
#define GLWINDOW_H

class TestDescriptor;
typedef struct _GLBEgl GLBEgl;

///The abstract base class of all window classes.
class GLWindow
{
public:
	virtual	bool Create () = 0;
	virtual	bool Destroy () = 0;
	virtual bool HandleMessage () = 0;
	virtual bool Done () = 0;
	virtual void ClearBuffers ();
	virtual	bool SwapBuffers () = 0;

	virtual void* Handle () = 0;
	virtual GLBEgl*	Egl() = 0;
	unsigned int GetActiveConfigID()
	{
		return m_activeConfigID;
	}

	virtual int GetMousePositionX(){return 0;}
	virtual int GetMousePositionY(){return 0;}

	virtual bool IsKeyDown(int id);
	virtual bool IsKeyUp(int id);
	virtual bool WasKeyPressed(int id);
	virtual bool WasKeyReleased(int id);
	virtual bool GetNewWidthHeight(int& w, int& h) const;//TODO depricated

	virtual void Resize(int x, int y);
	

	GLWindow();
	virtual ~GLWindow();
protected:
	int	m_activeConfigID;
private:
	
};

GLWindow *NewWindow (const char* glbversion);


inline void GLWindow::Resize(int x, int y)
{

}



inline GLWindow::GLWindow()
{
	
}

inline GLWindow::~GLWindow()
{
}


inline bool GLWindow::IsKeyDown(int id)
{
	return false;
}


inline bool GLWindow::IsKeyUp(int id)
{
	return false;
}


inline bool GLWindow::WasKeyPressed(int id)
{
	return false;
}


inline bool GLWindow::WasKeyReleased(int id)
{
	return false;
}


inline bool GLWindow::GetNewWidthHeight(int& w, int& h) const
{
	w = 0;
	h = 0;
	return false;
}

#endif
