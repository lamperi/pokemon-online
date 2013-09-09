#import "SoundDelegate.h"
#import <Foundation/NSObject.h>
#import <AppKit/NSSound.h>

#include <QDebug>

@implementation SoundDelegate

- (id) initWithCallback:(MacCrySupport*)callbackHandler WithId:(int)myId
{
    self = [super init];
    if (self) {
        qDebug() << "Constructed SoundDelegate with handler";
        handler = callbackHandler;
        id = myId;
    }
    return self;
}

- (void) sound:(NSSound *)sound didFinishPlaying:(BOOL)finishedPlaying
{
    qDebug() << "Calling didFinishPlaying" << sound << finishedPlaying;
    if (handler && finishedPlaying == YES) {
        qDebug() << "Calling for handler!";
        handler->playingFinished(id);
    }
}

- (void) removeCallback
{
    qDebug() << "Calling removeCallback";
    handler = 0;
}

@end
