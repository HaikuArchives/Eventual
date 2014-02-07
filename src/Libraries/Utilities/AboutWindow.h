/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ABOUT_WINDOW_H_
#define _ABOUT_WINDOW_H_

#include <SupportDefs.h>
#include <Window.h>

class AboutView;

class AboutWindow
	:
	public BWindow
{
public:
	AboutWindow();
	~AboutWindow();
private:
	AboutView* fAboutView;
};

#endif // _ABOUT_WINDOW_H_
