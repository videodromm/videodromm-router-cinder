#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Settings
#include "VDSettings.h"
// Session
#include "VDSession.h"
// Log
#include "VDLog.h"
// Spout
#include "CiSpoutOut.h"
#include "CiSpoutIn.h"
// NDI
#include "CinderNDIReceiver.h"
// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace videodromm;

class VDRouterApp : public App {

public:
	VDRouterApp();
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void toggleCursorVisibility(bool visible);
private:
	// Settings
	VDSettingsRef					mVDSettings;
	// Session
	VDSessionRef					mVDSession;
	// Log
	VDLogRef						mVDLog;
	// UI
	VDUIRef							mVDUI;
	// handle resizing for imgui
	void							resizeWindow();
	//string							mError;
	// fbo
	bool							mIsShutDown;
	Anim<float>						mRenderWindowTimer;
	void							positionRenderWindow();
	bool							mFadeInDelay;
	SpoutOut 						mSpoutOut;
	SpoutIn							mSpoutIn;
	// NDI
	CinderNDIReceiver				mReceiver;
	string							first = "";
	long							second = 0;
	bool							mFlipV;
	bool							mFlipH;
	int								xLeft, xRight, yLeft, yRight;
	int								margin, tWidth, tHeight;
	int								mode;
};


VDRouterApp::VDRouterApp()
	: mSpoutOut("Router", app::getWindowSize())
	, mReceiver{}
{
	// Settings
	mVDSettings = VDSettings::create("Router");
	// Session
	mVDSession = VDSession::create(mVDSettings);
	//mVDSettings->mCursorVisible = true;
	mVDSession->getWindowsResolution();
	toggleCursorVisibility(mVDSettings->mCursorVisible);
	mVDSession->toggleUI();
	mVDSession->setMode(1);

	mFadeInDelay = true;
	// UI
	mVDUI = VDUI::create(mVDSettings, mVDSession);
	// NDI
	mReceiver.setup();
	// windows
	mIsShutDown = false;
	mFlipV = false;
	mFlipH = true;
	xLeft = 0;
	xRight = mVDSettings->mRenderWidth;
	yLeft = 0;
	yRight = mVDSettings->mRenderHeight;
	margin = 20;
	tWidth = mVDSettings->mFboWidth / 2;
	tHeight = mVDSettings->mFboHeight / 2;
	mRenderWindowTimer = 0.0f;
	mode = 0;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });
	mVDSession->setBoolUniformValueByIndex(mVDSettings->IFLIPH, true);
	mVDSession->setBoolUniformValueByIndex(mVDSettings->IFLIPV, false);
}
void VDRouterApp::resizeWindow()
{
	mVDUI->resize();
}
void VDRouterApp::positionRenderWindow() {
	mVDSettings->mRenderPosXY = ivec2(mVDSettings->mRenderX, mVDSettings->mRenderY);//20141214 was 0
	setWindowPos(mVDSettings->mRenderX, mVDSettings->mRenderY);
	setWindowSize(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight);
}
void VDRouterApp::toggleCursorVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}
void VDRouterApp::fileDrop(FileDropEvent event)
{
	mVDSession->fileDrop(event);
}
void VDRouterApp::update()
{
	mVDSession->setFloatUniformValueByIndex(mVDSettings->IFPS, getAverageFps());
	mVDSession->update();
	if(mSpoutIn.getSize() != app::getWindowSize()) {
		//app::setWindowSize(mSpoutIn.getSize());
	}
	mReceiver.update();
	if (mReceiver.isReady()) {
		int senderIndex = mReceiver.getCurrentSenderIndex();
		int senderCount = mReceiver.getNumberOfSendersFound();
		std::string senderName = mReceiver.getCurrentSenderName();
		getWindow()->setTitle("#" + std::to_string(senderIndex) + "/" + std::to_string(senderCount) + ": " + senderName + " @ " + std::to_string((int)getAverageFps()) + " fps " + first + " - " + std::to_string(second));
	}
	else {
		getWindow()->setTitle(std::to_string((int)getAverageFps()) + " fps connecting...");
	}
}
void VDRouterApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save settings
		mVDSettings->save();
		mVDSession->save();
		quit();
	}
}
void VDRouterApp::mouseMove(MouseEvent event)
{
	if (!mVDSession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void VDRouterApp::mouseDown(MouseEvent event)
{
	if (event.isRightDown()) { // Select a sender
		// SpoutPanel.exe must be in the executable path
		mSpoutIn.getSpoutReceiver().SelectSenderPanel(); // DirectX 11 by default
	}
	if (!mVDSession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) { 
		}
	}
}
void VDRouterApp::mouseDrag(MouseEvent event)
{
	if (!mVDSession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}	
}
void VDRouterApp::mouseUp(MouseEvent event)
{
	if (!mVDSession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void VDRouterApp::keyDown(KeyEvent event)
{
#if defined( CINDER_COCOA )
	bool isModDown = event.isMetaDown();
#else // windows
	bool isModDown = event.isControlDown();
#endif
	CI_LOG_V("main keydown: " + toString(event.getCode()) + " ctrl: " + toString(isModDown));
	if (isModDown) {
	}
	if (!mVDSession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_c:
			// mouse cursor and ui visibility
			mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
			toggleCursorVisibility(mVDSettings->mCursorVisible);
			break;
	
		default:
			CI_LOG_V("main keydown: " + toString(event.getCode()));
			break;
		}
	}
}
void VDRouterApp::keyUp(KeyEvent event)
{
	if (!mVDSession->handleKeyUp(event)) {
		switch (event.getCode()) {
		
		default:
			CI_LOG_V("main keyup: " + toString(event.getCode()));
			break;
		}
	}
}

void VDRouterApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mVDSession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	
	// 20190215 gl::setMatricesWindow(toPixels(getWindowSize()), false);
		// 20190215 gl::setMatricesWindow(mVDSettings->mRenderWidth, mVDSettings->mRenderHeight, mVDSession->isFlipV());
	// 20190729 gl::setMatricesWindow(mVDSettings->mFboWidth, mVDSettings->mFboHeight, false);
	// must match windowSize
	gl::setMatricesWindow(mVDSession->getIntUniformValueByIndex(mVDSettings->IOUTW), mVDSession->getIntUniformValueByIndex(mVDSettings->IOUTH), false); 

	mode = mVDSession->getMode();
	/*switch (mode)
	{
	case 1:
		gl::draw(mVDSession->getMixTexture(), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getMixTexture());
		break;
	case 2:
		gl::draw(mVDSession->getRenderTexture(), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getRenderTexture());
		break;
	case 3:
		gl::draw(mVDSession->getHydraTexture(), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getHydraTexture());
		break;
	case 4:
		gl::draw(mVDSession->getFboTexture(0), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getFboTexture(0));
		break;
	case 5:
		gl::draw(mVDSession->getFboTexture(1), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getFboTexture(1));
		break;
	case 6:
		gl::draw(mVDSession->getFboTexture(2), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getFboTexture(2));
		break;
	case 7:
		gl::draw(mVDSession->getFboTexture(3), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getFboTexture(3));
		break;
	default:
		gl::draw(mVDSession->getMixetteTexture(), getWindowBounds());
		mSpoutOut.sendTexture(mVDSession->getMixetteTexture());
		break;
	} 
	*/
	
	//Rectf rectangle = Rectf(mVDSettings->mxLeft, mVDSettings->myLeft, mVDSettings->mxRight, mVDSettings->myRight);
	//gl::draw(mVDSession->getMixTexture(), rectangle);
	//gl::drawString("xRight: " + std::to_string(mVDSettings->myRight), vec2(toPixels(400), toPixels(300)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
	// 20190215 gl::setMatricesWindow(mVDSettings->mMainWindowWidth, mVDSettings->mMainWindowHeight, false);
	//gl::draw(mVDSession->getMixTexture(), getWindowBounds());
	
	
	// Spout Send
	//mSpoutOut.sendViewport();
	
	gl::draw(mVDSession->getMixetteTexture(), Rectf(0, 0, tWidth, tHeight));
	gl::drawString("Mixette", vec2(toPixels(xLeft), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipH MODE_IMAGE = 1
	gl::draw(mVDSession->getRenderTexture(), Rectf(tWidth * 2 + margin, 0, tWidth + margin, tHeight));
	gl::drawString("Render", vec2(toPixels(xLeft + tWidth + margin), toPixels(tHeight)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// flipV MODE_MIX = 0
	gl::draw(mVDSession->getMixTexture(), Rectf(0, tHeight * 2 + margin, tWidth, tHeight + margin));
	gl::drawString("Mix", vec2(toPixels(xLeft), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	// show the FBO color texture 
	gl::draw(mVDSession->getHydraTexture(), Rectf(tWidth + margin, tHeight + margin, tWidth * 2 + margin, tHeight * 2 + margin));
	gl::drawString("Hydra", vec2(toPixels(xLeft + tWidth + margin), toPixels(tHeight * 2 + margin)), Color(1, 1, 1), Font("Verdana", toPixels(16)));
	if (mReceiver.isReady()) {
		auto meta = mReceiver.getMetadata();
		auto ndiTex = mReceiver.getVideoTexture();

		first = meta.first;
		second = meta.second;
		if (ndiTex.first) {

			Rectf centeredRect = Rectf(ndiTex.first->getBounds()).getCenteredFit(getWindowBounds(), true);
			gl::draw(ndiTex.first, centeredRect);
			/*for (auto &warp : mWarps) {
				warp->draw(ndiTex.first, ndiTex.first->getBounds());
			}*/
		}
	}

	gl::drawString("irx: " + std::to_string(mVDSession->getFloatUniformValueByName("iResolutionX"))
		+ " iry: " + std::to_string(mVDSession->getFloatUniformValueByName("iResolutionY"))
		+ " fw: " + std::to_string(mVDSettings->mFboWidth)
		+ " fh: " + std::to_string(mVDSettings->mFboHeight),
		vec2(xLeft, getWindowHeight() - toPixels(30)), Color(1, 1, 1),
		Font("Verdana", toPixels(24)));

	// imgui
	if (mVDSession->showUI()) {
		mVDUI->Run("UI", (int)getAverageFps());
		if (mVDUI->isReady()) {
		}
	}
	//getWindow()->setTitle(mVDSettings->sFps + " fps VDRouter");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
	//settings->setWindowSize(1920, 1080);
#ifdef _DEBUG
	//settings->setConsoleWindowEnabled();
#else
	//settings->setConsoleWindowEnabled();
#endif  // _DEBUG
}

CINDER_APP(VDRouterApp, RendererGl, prepareSettings)
