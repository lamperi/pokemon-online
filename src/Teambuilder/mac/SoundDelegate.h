#ifndef SOUNDDELEGATE_H
#define SOUNDDELEGATE_H

#import <Foundation/NSObject.h>
#import <AppKit/NSSound.h>

#include "MacCrySupport.h"

@interface SoundDelegate : NSObject <NSSoundDelegate> {
    MacCrySupport *handler;
    int id;
    int type;
}

- (id) initWithCallback:(MacCrySupport*) callbackHandler WithId:(int)id WithType:(int)type;
- (void) sound:(NSSound *)sound didFinishPlaying:(BOOL)finishedPlaying;
- (void) removeCallback;

@end

#endif
