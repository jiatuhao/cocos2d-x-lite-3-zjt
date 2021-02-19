/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2013-2017 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#import "Application.h"
#import <UIKit/UIKit.h>
#include "base/Scheduler.h"
#include "base/AutoreleasePool.h"
#include "bindings/event/EventDispatcher.h"
#include "bindings/jswrapper/SeApi.h"
#include "audio/include/AudioEngine.h"
#include "platform/Device.h"

#ifndef CC_USE_METAL
@interface MyTimer : NSObject {
    cc::Application *_app;
    CADisplayLink *_displayLink;
    int _fps;
}
- (instancetype)initWithApp:(cc::Application *)app fps:(int)fps;
- (void)start;
- (void)changeFPS:(int)fps;
- (void)pause;
- (void)resume;
@end

@implementation MyTimer

- (instancetype)initWithApp:(cc::Application *)app fps:(int)fps {
    if (self = [super init]) {
        _fps = fps;
        _app = app;
        _displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(renderScene:)];
        _displayLink.preferredFramesPerSecond = _fps;
    }
    return self;
}

- (void)start {
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)pause {
    _displayLink.paused = TRUE;
}

- (void)resume {
    _displayLink.paused = FALSE;
}

- (void)changeFPS:(int)fps {
    _displayLink.preferredFramesPerSecond = fps;
}

- (void)renderScene:(id)sender {
    _app->tick();
}

@end
#endif

namespace cc {

namespace {
bool setCanvasCallback(se::Object *global) {
    auto viewLogicalSize = cc::Application::getInstance()->getViewLogicalSize();

    CGRect nativeBounds = [[UIScreen mainScreen] nativeBounds];
    int nativeWidth = static_cast<int>(nativeBounds.size.width);
    int nativeHeight = static_cast<int>(nativeBounds.size.height);
    auto orientation = cc::Device::getDeviceOrientation();
    bool isLandscape = (orientation == cc::Device::Orientation::LANDSCAPE_RIGHT || orientation == cc::Device::Orientation::LANDSCAPE_LEFT);
    if (isLandscape) std::swap(nativeWidth, nativeHeight);

    char commandBuf[200] = {0};
    // https://stackoverflow.com/questions/5795978/string-format-for-intptr-t-and-uintptr-t/41897226#41897226
    // format intptr_t
    //set window.innerWidth/innerHeight in css pixel units
    sprintf(commandBuf, "window.innerWidth = %d; window.innerHeight = %d; window.nativeWidth = %d; window.nativeHeight = %d; window.windowHandler = 0x%" PRIxPTR ";",
            static_cast<int>(viewLogicalSize.x),
            static_cast<int>(viewLogicalSize.y),
            nativeWidth,
            nativeHeight,
            reinterpret_cast<uintptr_t>(UIApplication.sharedApplication.delegate.window.rootViewController.view));

    se::ScriptEngine *se = se::ScriptEngine::getInstance();
    se->evalString(commandBuf);
    return true;
}

#ifndef CC_USE_METAL
MyTimer *_timer;
#endif
} // namespace

Application *Application::_instance = nullptr;
std::shared_ptr<Scheduler> Application::_scheduler = nullptr;

Application::Application(int width, int height) {
    Application::_instance = this;
    _scheduler = std::make_shared<Scheduler>();
    EventDispatcher::init();
    _viewLogicalSize.x = width;
    _viewLogicalSize.y = height;

#ifndef CC_USE_METAL
    _timer = [[MyTimer alloc] initWithApp:this fps:_fps];
#endif
}

Application::~Application() {
#if USE_AUDIO
    AudioEngine::end();
#endif

    EventDispatcher::destroy();
    se::ScriptEngine::destroyInstance();

    Application::_instance = nullptr;

#ifndef CC_USE_METAL
    [_timer release];
#endif
}

std::string Application::getCurrentLanguageCode() const {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];
    return [currentLanguage UTF8String];
}

bool Application::isDisplayStats() {
    se::AutoHandleScope hs;
    se::Value ret;
    char commandBuf[100] = "cc.debug.isDisplayStats();";
    se::ScriptEngine::getInstance()->evalString(commandBuf, 100, &ret);
    return ret.toBoolean();
}

void Application::setDisplayStats(bool isShow) {
    se::AutoHandleScope hs;
    char commandBuf[100] = {0};
    sprintf(commandBuf, "cc.debug.setDisplayStats(%s);", isShow ? "true" : "false");
    se::ScriptEngine::getInstance()->evalString(commandBuf);
}

void Application::setCursorEnabled(bool value) {
}

Application::LanguageType Application::getCurrentLanguage() const {
    // get the current language and country config
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];

    // get the current language code.(such as English is "en", Chinese is "zh" and so on)
    NSDictionary *temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
    NSString *languageCode = [temp objectForKey:NSLocaleLanguageCode];

    if ([languageCode isEqualToString:@"zh"]) return LanguageType::CHINESE;
    if ([languageCode isEqualToString:@"en"]) return LanguageType::ENGLISH;
    if ([languageCode isEqualToString:@"fr"]) return LanguageType::FRENCH;
    if ([languageCode isEqualToString:@"it"]) return LanguageType::ITALIAN;
    if ([languageCode isEqualToString:@"de"]) return LanguageType::GERMAN;
    if ([languageCode isEqualToString:@"es"]) return LanguageType::SPANISH;
    if ([languageCode isEqualToString:@"nl"]) return LanguageType::DUTCH;
    if ([languageCode isEqualToString:@"ru"]) return LanguageType::RUSSIAN;
    if ([languageCode isEqualToString:@"ko"]) return LanguageType::KOREAN;
    if ([languageCode isEqualToString:@"ja"]) return LanguageType::JAPANESE;
    if ([languageCode isEqualToString:@"hu"]) return LanguageType::HUNGARIAN;
    if ([languageCode isEqualToString:@"pt"]) return LanguageType::PORTUGUESE;
    if ([languageCode isEqualToString:@"ar"]) return LanguageType::ARABIC;
    if ([languageCode isEqualToString:@"nb"]) return LanguageType::NORWEGIAN;
    if ([languageCode isEqualToString:@"pl"]) return LanguageType::POLISH;
    if ([languageCode isEqualToString:@"tr"]) return LanguageType::TURKISH;
    if ([languageCode isEqualToString:@"uk"]) return LanguageType::UKRAINIAN;
    if ([languageCode isEqualToString:@"ro"]) return LanguageType::ROMANIAN;
    if ([languageCode isEqualToString:@"bg"]) return LanguageType::BULGARIAN;
    return LanguageType::ENGLISH;
}

Application::Platform Application::getPlatform() const {
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) // idiom for iOS <= 3.2, otherwise: [UIDevice userInterfaceIdiom] is faster.
        return Platform::IPAD;
    else
        return Platform::IPHONE;
}

bool Application::openURL(const std::string &url) {
    NSString *msg = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    NSURL *nsUrl = [NSURL URLWithString:msg];
    return [[UIApplication sharedApplication] openURL:nsUrl];
}

void Application::copyTextToClipboard(const std::string &text) {
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    pasteboard.string = [NSString stringWithCString:text.c_str() encoding:NSUTF8StringEncoding];
}

bool Application::init() {
    se::ScriptEngine *se = se::ScriptEngine::getInstance();
    se->addRegisterCallback(setCanvasCallback);

#ifndef CC_USE_METAL
    [_timer start];
#endif

    return true;
}

void Application::onPause() {
#ifndef CC_USE_METAL
    [_timer pause];
#endif
}

void Application::onResume() {
#ifndef CC_USE_METAL
    [_timer resume];
#endif
}

std::string Application::getSystemVersion() {
    NSString *systemVersion = [UIDevice currentDevice].systemVersion;
    return [systemVersion UTF8String];
}

void Application::setPreferredFramesPerSecond(int fps) {
    _fps = fps;
#ifndef CC_USE_METAL
    [_timer changeFPS:_fps];
#endif
}

} // namespace cc
