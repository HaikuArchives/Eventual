/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// System includes
#include <InterfaceDefs.h>
#include <Rect.h>
#include <View.h>
 
// Project includes
#include "AboutWindow.h"
#include "AboutView.h"
#include "URLView.h"



/*!	\brief		Constructor for class AboutWindow.
 *		\details		Constructs a window to show the view with "about" info inside.
 */
AboutWindow::AboutWindow()
	:
	BWindow( BRect( 0, 0, 639, 479 ),
				"About Eventual",
				B_TITLED_WINDOW,
				B_NOT_ZOOMABLE | B_NOT_RESIZABLE,
				B_ALL_WORKSPACES )
{
	fAboutView = new AboutView( Bounds().InsetBySelf( 1, 1 ) );
	if ( fAboutView ) {
		this->AddChild( fAboutView );
		fAboutView->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	}
	this->CenterOnScreen();
	this->Show();
}	// <-- end of constructor for AboutWindow



/*!	\brief		Destructor for the AboutWindow.
 */
AboutWindow::~AboutWindow()
{
	if ( fAboutView ) {
		fAboutView->RemoveSelf();
		delete fAboutView;
	}
}	// <-- end of destructor for class AboutWindow
