#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "u_mem.h"
#include "console.h"
#include "descent.h"
#include "grdef.h"
#include "ogl_render.h"
#include "canvas.h"

fix CCanvas::xCanvW2;
fix CCanvas::xCanvH2;
float CCanvas::fCanvW2;
float CCanvas::fCanvH2;

CStack<tCanvasBackup> CCanvas::m_save;
float CScreen::m_fScale = 1.0f;

//CScreen screen;

//	-----------------------------------------------------------------------------

CCanvas *CCanvas::Create (int32_t w, int32_t h)
{
	CCanvas *canvP;

if ((canvP = new CCanvas)) 
	canvP->Setup (w, h);
return canvP;
}

//	-----------------------------------------------------------------------------

void CCanvas::Init (void)
{
memset (&m_info, 0, sizeof (m_info));
if (!m_save.Buffer ())
	m_save.Create (10);
m_parent = NULL;
SetName ("Canvas");
}

//	-----------------------------------------------------------------------------

void CCanvas::Setup (int32_t w, int32_t h)
{
Init ();
Setup (NULL, 0, 0, w, h);
//CBitmap::Setup (BM_LINEAR, w, h, 1, "Canvas");
}

//	-----------------------------------------------------------------------------

CCanvas *CCanvas::CreatePane (int32_t x, int32_t y, int32_t w, int32_t h)
{
	CCanvas *paneP;

if (!(paneP = new CCanvas))	
	return NULL;
//SetupPane (paneP, x, y, w, h);
paneP->Setup (this, x, y, w, h);
return paneP;
}

//	-----------------------------------------------------------------------------

void CCanvas::Init (int32_t nType, int32_t w, int32_t h, uint8_t *data)
{
Init ();
//CBitmap::Init (nType, 0, 0, w, h, 1, data);
}

//	-----------------------------------------------------------------------------

void CCanvas::Setup (CCanvas* parentP)
{
SetColor (parentP->m_info.color);
SetDrawMode (parentP->m_info.nDrawMode);
SetFont (parentP->m_info.font ? parentP->m_info.font : GAME_FONT);
SetFontColor (parentP->m_info.fontColors [0], 0);
SetFontColor (parentP->m_info.fontColors [1], 1);
*((CViewport*) this) = *((CViewport*) parentP);
SetWidth ();
SetHeight ();
}

//	-----------------------------------------------------------------------------

void CCanvas::Setup (CCanvas* parentP, int32_t x, int32_t y, int32_t w, int32_t h, bool bUnscale)
{
if (parentP)
	Setup (parentP);
*((CViewport*) this) = bUnscale ? CViewport (Unscaled (x), Unscaled (y), Unscaled (w), Unscaled (h)) : CViewport (x, y, w, h);
SetWidth ();
SetHeight ();
}

//	-----------------------------------------------------------------------------

void CCanvas::Destroy (void)
{
Deactivate ();
delete this;
}

//	-----------------------------------------------------------------------------

void CCanvas::Clear (uint32_t color)
{
SetColorRGBi (color);
OglDrawFilledRect (0, 0, CCanvas::Current ()->Width () - 1, CCanvas::Current ()->Height () - 1);
}

//	-----------------------------------------------------------------------------

void CCanvas::SetColor (int32_t color)
{
m_info.color.index =color % 256;
m_info.color.rgb = 0;
}

//	-----------------------------------------------------------------------------

void CCanvas::SetColorRGB (uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
m_info.color.rgb = 1;
m_info.color.Red () = red;
m_info.color.Green () = green;
m_info.color.Blue () = blue;
m_info.color.Alpha () = alpha;
}

//	-----------------------------------------------------------------------------

void CCanvas::SetColorRGB15bpp (uint16_t c, uint8_t alpha)
{
CCanvas::SetColorRGB (
	PAL2RGBA (((c >> 10) & 31) * 2), 
	PAL2RGBA (((c >> 5) & 31) * 2), 
	PAL2RGBA ((c & 31) * 2), 
	alpha);
}

//	-----------------------------------------------------------------------------

void CCanvas::FadeColorRGB (double dFade)
{
if (dFade && m_info.color.rgb) {
	m_info.color.Red () = (uint8_t) (m_info.color.Red () * dFade);
	m_info.color.Green () = (uint8_t) (m_info.color.Green () * dFade);
	m_info.color.Blue () = (uint8_t) (m_info.color.Blue () * dFade);
	//m_info.color.Alpha () = (uint8_t) ((float) gameStates.render.grAlpha / (float) FADE_LEVELS * 255.0f);
	}
}

//	-----------------------------------------------------------------------------

void CCanvas::GetExtent (CRectangle& rc, bool bScale, bool bDeep)
{
	CCanvas* parent = m_parent;

if (!(bDeep && parent))
	rc = *((CRectangle*) this);
else {
	int32_t l = Left (false);
	int32_t t = Top (false);
	int32_t w = Width (false);
	int32_t h = Height (false);
	while (parent) {
		l += parent->Left ();
		t += parent->Top ();
		if (!w)
			w = parent->Width ();
		if (!h)
			h = parent->Height ();
		parent = parent->Parent ();
		}
	rc = CRectangle (l, t, w, h);
	}
if (bScale) {
	rc.SetLeft (Scaled (rc.Left ()));
	rc.SetTop (Scaled (rc.Top ()));
	rc.SetWidth (Scaled (rc.Width ()));
	rc.SetHeight (Scaled (rc.Width ()));
	}	
}

//	-----------------------------------------------------------------------------

void CCanvas::SetViewport (CCanvas* parent)
{
if (!parent)
	ogl.SetViewport (Left (), Top (), Width (), Height ());
else {
	m_parent = parent;
	int32_t l = Left ();
	int32_t t = Top ();
	int32_t w = Width ();
	int32_t h = Height ();
	while (parent) {
		l += parent->Left ();
		t += parent->Top ();
		if (!w)
			w = parent->Width ();
		if (!h)
			h = parent->Height ();
		parent = parent->Parent ();
		}
	ogl.SetViewport (l, t, w, h);
	}
}

//	-----------------------------------------------------------------------------

void CCanvas::Activate (const char* szId, CCanvas* parent, bool bReset) 
{
if (bReset) {
	m_save.Reset ();
	fontManager.Reset ();
	}
//if (CCanvas::Current () != this) 
	{
#if DBG
	SetId (szId);
#endif
	CCanvas::Push (this);
	}
SetViewport (parent);
}

//	-----------------------------------------------------------------------------

void CCanvas::Deactivate (void) 
{ 
CCanvas* canvP;
do {
	if (!(canvP = Current ()))
		break;
	Pop ();
} while (canvP != this);
if (Current ())
	Current ()->Reactivate ();
else
	gameData.render.frame.Activate ("Deactivate (frame)");
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//------------------------------------------------------------------------------

void SetupCanvasses (float scale)
{
if (scale != 0.0f)
	gameData.render.screen.SetScale ((scale < 0.0f) ? ogl.IsOculusRift () ? 1.25f : 1.0f : scale);

if (!ogl.IsSideBySideDevice ())
	gameData.render.frame.Setup (&gameData.render.screen);
else {
	if (ogl.StereoSeparation () < 0)
		gameData.render.frame.Setup (&gameData.render.screen, 0, 0, gameData.render.screen.Width (false) / 2, gameData.render.screen.Height (false));
	else
		gameData.render.frame.Setup (&gameData.render.screen, gameData.render.screen.Width (false) / 2, 0, gameData.render.screen.Width (false) / 2, gameData.render.screen.Height (false));
	}
gameData.render.scene.Setup (&gameData.render.frame);
gameData.render.window.Setup (&gameData.render.scene);
//gameData.render.screen.SetScale (1.0f);
}

//	-----------------------------------------------------------------------------
//eof
