/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

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
#include "audio/include/AudioEngine.h"
#include "base/Scheduler.h"
#include "cocos/bindings/jswrapper/SeApi.h"
#include "platform/Application.h"
#include <algorithm>
#include <mutex>

#import <AppKit/AppKit.h>

@interface MyTimer : NSObject {
    cc::Application *_app;
    NSTimer *_timer;
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
    }
    return self;
}

- (void)start {
    _timer = [NSTimer scheduledTimerWithTimeInterval:1.0f / _fps
                                              target:self
                                            selector:@selector(renderScene)
                                            userInfo:nil
                                             repeats:YES];
}

- (void)pause {
    [_timer invalidate];
}

- (void)resume {
    [self start];
}

- (void)changeFPS:(int)fps {
    if (fps == _fps)
        return;

    [self pause];
    [self resume];
}

- (void)renderScene {
    _app->tick();
}

@end

namespace cc {

namespace {
bool setCanvasCallback(se::Object *global) {
    auto viewLogicalSize = Application::getInstance()->getViewLogicalSize();
    se::ScriptEngine *se = se::ScriptEngine::getInstance();
    char commandBuf[200] = {0};
    NSView *view = [[[[NSApplication sharedApplication] delegate] getWindow] contentView];
    sprintf(commandBuf, "window.innerWidth = %d; window.innerHeight = %d; window.windowHandler = 0x%" PRIxPTR ";",
            (int)(viewLogicalSize.x),
            (int)(viewLogicalSize.y),
            (uintptr_t)view);
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

    _viewLogicalSize.x = width;
    _viewLogicalSize.y = height;

    _scheduler = std::make_shared<Scheduler>();
    EventDispatcher::init();

#ifndef CC_USE_METAL
    _timer = [[MyTimer alloc] initWithApp:this fps:_fps];
#endif
    
    [[[[[NSApplication sharedApplication] delegate] getWindow] contentView] start];
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

bool Application::init() {
    se::ScriptEngine *se = se::ScriptEngine::getInstance();
    se->addRegisterCallback(setCanvasCallback);

#ifndef CC_USE_METAL
    [_timer start];
#endif

    return true;
}

void Application::setPreferredFramesPerSecond(int fps) {
    _fps = fps;

#ifndef CC_USE_METAL
    [_timer changeFPS:_fps];
#endif
}

Application::Platform Application::getPlatform() const {
    return Platform::MAC;
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
    if (value)
        CGDisplayShowCursor(kCGDirectMainDisplay);
    else
        CGDisplayHideCursor(kCGDirectMainDisplay);
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

bool Application::openURL(const std::string &url) {
    NSString *msg = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
    NSURL *nsUrl = [NSURL URLWithString:msg];
    return [[NSWorkspace sharedWorkspace] openURL:nsUrl];
}

void Application::copyTextToClipboard(const std::string &text) {
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    NSString *tmp = [NSString stringWithCString:text.c_str() encoding:NSUTF8StringEncoding];
    [pasteboard setString:tmp forType:NSStringPboardType];
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
    NSOperatingSystemVersion v = NSProcessInfo.processInfo.operatingSystemVersion;
    char version[50] = {0};
    snprintf(version, sizeof(version), "%d.%d.%d", (int)v.majorVersion, (int)v.minorVersion, (int)v.patchVersion);
    return version;
}

} // namespace cc
