//
//  ViewController.m
//  Audio Controller Test Suite
//
//  Created by Michael Tyson on 13/02/2012.
//  Copyright (c) 2012 A Tasty Pixel. All rights reserved.
//

#import "ViewController.h"
#import "TheAmazingAudioEngine.h"
#import "TPOscilloscopeLayer.h"
#import "AEPlaythroughChannel.h"
#import "AEExpanderFilter.h"
#import "AELimiterFilter.h"
#import "Reverberation.hpp"
#import <QuartzCore/QuartzCore.h>
#import "FDN.h"
#import "MultiLevelBiQuadFilter.h"
#import "DoubleBufferedReverb.h"

#include <math.h>

static int kInputChannelsChangedContext;

#define kAuxiliaryViewTag 251


@interface ViewController () {
    AudioFileID _audioUnitFile;
    AEChannelGroupRef _group;
   // DoubleBufferedReverb Reverb;
    Reverberation Reverb;
    MultiLevelBiQuadFilter equalizer;
    UIButton *Listener;
    UIButton *SoundSource;
    bool autoSoundMove;
    float angle;
    int counter;

    bool ssLock;
    float rSize;
    float rt60Val;
    float wRatio;
    
    bool reverbOn;
    bool directOn;
    
    float xDiff;
    float yDiff;
    
    float xLloc;
    float yLloc;
    

    

}
@property (nonatomic, strong) AEAudioController *audioController;
@property (nonatomic, strong) AEAudioFilePlayer *loop1;
@property (nonatomic, strong) AEAudioFilePlayer *loop2;
@property (nonatomic, strong) AEAudioFilePlayer *loop3;
@property (nonatomic, strong) AEAudioFilePlayer *loop4;
@property (nonatomic, strong) AEBlockChannel *oscillator;
@property (nonatomic, strong) AEAudioUnitChannel *audioUnitPlayer;
@property (nonatomic, strong) AEPlaythroughChannel *playthrough;
@property (nonatomic, strong) AELimiterFilter *limiter;
@property (nonatomic, strong) AEExpanderFilter *expander;
@property (nonatomic, strong) AEBlockFilter *reverbBlock;
@property (nonatomic, strong) TPOscilloscopeLayer *outputOscilloscope;
@property (nonatomic, strong) TPOscilloscopeLayer *inputOscilloscope;
@property (nonatomic, strong) CALayer *inputLevelLayer;
@property (nonatomic, strong) CALayer *outputLevelLayer;
@property (nonatomic, weak) NSTimer *levelsTimer;
@property  float number;
@property (nonatomic, weak) NSTimer *soundSourceTimer;
@property (nonatomic, weak) NSTimer *changeListenerTimer;
@end

@implementation ViewController


- (id)initWithAudioController:(AEAudioController*)audioController {
    if ( !(self = [super initWithStyle:UITableViewStyleGrouped]) ) return nil;
    
   // done = true;
    
    reverbOn = true;
    directOn = true;
    
    ssLock = false;
    
    counter = 0;
    self.number = 1.0f;
    
    
    rSize = 0.15f;
    wRatio = 0.5f;
    rt60Val = 0.4f;
    
    autoSoundMove = false;
    angle = 0.0f;
        self.soundSourceTimer = [NSTimer scheduledTimerWithTimeInterval:5.0 target:self selector:@selector(moveSoundSource:) userInfo:nil repeats:YES];

    
    xLloc = self.tableView.bounds.size.width/2-25;
    yLloc = 2*self.tableView.bounds.size.width/3;
    
    xDiff = self.tableView.bounds.size.width/2-25 - xLloc;
    yDiff = self.tableView.bounds.size.width/3 - yLloc;
    
    Listener = [UIButton buttonWithType:UIButtonTypeCustom];
    [Listener addTarget:self action:@selector(listenerTouched:withEvent:) forControlEvents:UIControlEventTouchDragInside|UIControlEventTouchDragOutside];
    [Listener addTarget:self action:@selector(listenerTouchedEnds:withEvent:) forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
    [Listener setFrame:CGRectMake(self.tableView.bounds.size.width/2-25, 2*self.tableView.bounds.size.width/3, 50, 50)];
    UIImage *listenerImage = [UIImage imageNamed:@"person.png"];
    [Listener setImage:listenerImage forState:UIControlStateNormal];
    
    SoundSource = [UIButton buttonWithType:UIButtonTypeCustom];
    [SoundSource addTarget:self action:@selector(soundSourceTouched:withEvent:) forControlEvents:UIControlEventTouchDragInside| UIControlEventTouchDragOutside];
    [SoundSource addTarget:self action:@selector(soundSourceTouchedEnds:withEvent:) forControlEvents:UIControlEventTouchUpOutside | UIControlEventTouchUpInside];
    
    [SoundSource setFrame:CGRectMake(self.tableView.bounds.size.width/2-25, self.tableView.bounds.size.width/3, 50, 50)];
    UIImage *soundSourceImage = [UIImage imageNamed:@"Speaker.png"];
    UIImage *soundSourceImageTouched = [UIImage imageNamed:@"SpeakerTouched.png"];
    [SoundSource setImage:soundSourceImage  forState:UIControlStateNormal];
    [SoundSource setImage:soundSourceImageTouched forState:UIControlStateHighlighted];
    
    
    self.audioController = audioController;
    
    self.loop1 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"fullSpectrumImpulse" withExtension:@"wav"] error:NULL];
    _loop1.volume = 1.0;
    _loop1.channelIsMuted = YES;
    _loop1.loop = YES;
    
    self.loop2 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"acapellafemale2" withExtension:@"wav"] error:NULL];
    _loop2.volume = 1.0;
    _loop2.channelIsMuted = YES;
    _loop2.loop = YES;
    
    self.loop3 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"lowpassFilteredImpulse" withExtension:@"wav"] error:NULL];
    _loop3.volume = 1.0;
    _loop3.channelIsMuted = YES;
    _loop3.loop = YES;
    
    
    self.loop4 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"acapellafemale" withExtension:@"wav"] error:NULL];
    _loop4.volume = 1.0;
    _loop4.channelIsMuted = YES;
    _loop4.loop = YES;

    // Create a block-based channel, with an implementation of an oscillator
    
    __block float oscillatorPosition = 0;
    __block float oscillatorRate = 622.0/44100.0;
    self.oscillator = [AEBlockChannel channelWithBlock:^(const AudioTimeStamp  *time,
                                                               UInt32           frames,
                                                               AudioBufferList *audio) {
        for ( int i=0; i<frames; i++ ) {
            // Quick sin-esque oscillator
            float x = oscillatorPosition;
            x *= x; x -= 1.0; x *= x;       // x now in the range 0...1
            x *= INT16_MAX;
            x -= INT16_MAX / 2;
            oscillatorPosition += oscillatorRate;
            if ( oscillatorPosition > 1.0 ) oscillatorPosition -= 2.0;
            
            ((SInt16*)audio->mBuffers[0].mData)[i] = x;
            ((SInt16*)audio->mBuffers[1].mData)[i] = x;
        }
    }];
    _oscillator.audioDescription = [AEAudioController nonInterleaved16BitStereoAudioDescription];
    
    _oscillator.channelIsMuted = YES;
    
    _group = [_audioController createChannelGroup];
    [_audioController addChannels:@[ _loop1,_loop2,_loop3, _loop4, _oscillator] toChannelGroup:_group];
    
    [_audioController addObserver:self forKeyPath:@"numberOfInputChannels" options:0 context:(void*)&kInputChannelsChangedContext];
    _audioController.inputGain = 1.0f;
    return self;
}

-(void)dealloc {
    [_audioController removeObserver:self forKeyPath:@"numberOfInputChannels"];
    
    if ( _levelsTimer ) [_levelsTimer invalidate];

    NSMutableArray *channelsToRemove = [NSMutableArray arrayWithObjects:nil];
    
    if ( _playthrough ) {
        [channelsToRemove addObject:_playthrough];
        [_audioController removeInputReceiver:_playthrough];
    }
    
    [_audioController removeChannels:channelsToRemove];
    
    if ( _limiter ) {
        [_audioController removeFilter:_limiter];
    }
    
    if ( _expander ) {
        [_audioController removeFilter:_expander];
    }
    
    if ( _reverbBlock ) {
        [_audioController removeFilter:_reverbBlock];
    }
    
    if ( _audioUnitFile ) {
        AudioFileClose(_audioUnitFile);
    }
}

-(void)viewDidLoad {
    [super viewDidLoad];

    
    UIView *headerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.tableView.bounds.size.width, 100)];
    headerView.backgroundColor = [UIColor groupTableViewBackgroundColor];
    
    self.outputOscilloscope = [[TPOscilloscopeLayer alloc] initWithAudioController:_audioController];
    _outputOscilloscope.frame = CGRectMake(0, 0, headerView.bounds.size.width, 80);
    [headerView.layer addSublayer:_outputOscilloscope];
    [_audioController addOutputReceiver:_outputOscilloscope];
    [_outputOscilloscope start];
    
    self.inputOscilloscope = [[TPOscilloscopeLayer alloc] initWithAudioController:_audioController];
    _inputOscilloscope.frame = CGRectMake(0, 0, headerView.bounds.size.width, 80);
    _inputOscilloscope.lineColor = [UIColor colorWithWhite:0.0 alpha:0.3];
    [headerView.layer addSublayer:_inputOscilloscope];
    [_audioController addInputReceiver:_inputOscilloscope];
    [_inputOscilloscope start];
    
    self.inputLevelLayer = [CALayer layer];
    _inputLevelLayer.backgroundColor = [[UIColor colorWithWhite:0.0 alpha:0.3] CGColor];
    _inputLevelLayer.frame = CGRectMake(headerView.bounds.size.width/2.0 - 5.0 - (0.0), 90, 0, 10);
    [headerView.layer addSublayer:_inputLevelLayer];
    
    self.outputLevelLayer = [CALayer layer];
    _outputLevelLayer.backgroundColor = [[UIColor colorWithWhite:0.0 alpha:0.3] CGColor];
    _outputLevelLayer.frame = CGRectMake(headerView.bounds.size.width/2.0 + 5.0, 90, 0, 10);
    [headerView.layer addSublayer:_outputLevelLayer];
    
    self.tableView.tableHeaderView = headerView;
    
}


-(void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    self.levelsTimer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(updateLevels:) userInfo:nil repeats:YES];
}

-(void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    [_levelsTimer invalidate];
    self.levelsTimer = nil;
}

-(void)viewDidLayoutSubviews {
    _outputOscilloscope.frame = CGRectMake(0, 0, self.tableView.tableHeaderView.bounds.size.width, 80);
    _inputOscilloscope.frame = CGRectMake(0, 0, self.tableView.tableHeaderView.bounds.size.width, 80);
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation {
    return YES;
}

-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 5;
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    switch ( section ) {
        case 1:
            return 4;
            
        case 2:
            return 5;
            
        case 3:
            return 4 + (_audioController.numberOfInputChannels > 1 ? 1 : 0);
        case 4:
            return 1;
            
        default:
            return 0;
    }
}

- (void)loop2SwitchChanged:(UISwitch*)sender {
    _loop2.channelIsMuted = !sender.isOn;
}

- (void)loop2VolumeChanged:(UISlider*)sender {
    _loop2.volume = sender.value;
}

- (void)loop1SwitchChanged:(UISwitch*)sender {
    _loop1.channelIsMuted = !sender.isOn;
}

- (void)loop1VolumeChanged:(UISlider*)sender {
    _loop1.volume = sender.value;
}
- (void)loop3SwitchChanged:(UISwitch*)sender {
    _loop3.channelIsMuted = !sender.isOn;
}

- (void)loop3VolumeChanged:(UISlider*)sender {
    _loop3.volume = sender.value;
}
- (void)loop4SwitchChanged:(UISwitch*)sender {
    _loop4.channelIsMuted = !sender.isOn;
}

- (void)loop4VolumeChanged:(UISlider*)sender {
    _loop4.volume = sender.value;
}


-(UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    BOOL isiPad = [[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad;
    
    static NSString *cellIdentifier = @"cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if ( !cell ) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];
    }
    
    cell.accessoryView = nil;
    cell.selectionStyle = UITableViewCellSelectionStyleNone;
    [[cell viewWithTag:kAuxiliaryViewTag] removeFromSuperview];
    
    switch ( indexPath.section ) {
        case 0: {
    
            switch ( indexPath.row ) {
             
                case 0: {
                    cell.textLabel.text = @"High-pass Frequency";
                    UISlider *highpassSlider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    highpassSlider.minimumValue = 0.0;
                    highpassSlider.maximumValue = 500; // max fc = 500 hz
                    highpassSlider.value = 210;
                    equalizer.set1stOrderHighPass(highpassSlider.value);
                    [highpassSlider addTarget:self action:@selector(highpassFrequencyChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = highpassSlider;
                    break;
                }
                case 1: {
                    cell.textLabel.text = @"1000 Hz gain";
                    UISlider *bell2Slider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    bell2Slider.minimumValue = 0.1;
                    bell2Slider.maximumValue = 2.0;
                    bell2Slider.value = 0.5;
                    equalizer.setBell(1000.0, 500.0, bell2Slider.value, 2);
                    [bell2Slider addTarget:self action:@selector(bell2GainChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = bell2Slider;
                    break;
                }
                case 2: {
                    cell.textLabel.text = @"Low-pass Frequency";
                    UISlider *lowpassSlider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    lowpassSlider.minimumValue = 3000;
                    lowpassSlider.maximumValue = 22050;
                    lowpassSlider.value = 6000;
                    equalizer.setLowPass(lowpassSlider.value);
                    [lowpassSlider addTarget:self action:@selector(lowpassFrequencyChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = lowpassSlider;
                    break;
                }
            }
            break;
        }
            
            
        case 1: {
            UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 200, 40)];
            
            UISlider *slider = [[UISlider alloc] initWithFrame:CGRectZero];
            slider.translatesAutoresizingMaskIntoConstraints = NO;
            slider.maximumValue = 1.0;
            slider.minimumValue = 0.0;
            
            UISwitch * onSwitch = [[UISwitch alloc] initWithFrame:CGRectZero];
            onSwitch.translatesAutoresizingMaskIntoConstraints = NO;
            onSwitch.on = _expander != nil;
            [onSwitch addTarget:self action:@selector(expanderSwitchChanged:) forControlEvents:UIControlEventValueChanged];
            [view addSubview:slider];
            [view addSubview:onSwitch];
            [view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[slider]-20-[onSwitch]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(slider, onSwitch)]];
            [view addConstraint:[NSLayoutConstraint constraintWithItem:slider attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
            [view addConstraint:[NSLayoutConstraint constraintWithItem:onSwitch attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
            
            cell.accessoryView = view;
            switch ( indexPath.row ) {
                case 0: {
                    cell.textLabel.text= @"sinesweep";
                    onSwitch.on = !_loop3.channelIsMuted;
                    slider.value = _loop3.volume;
                    [onSwitch addTarget:self action:@selector(loop3SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop3VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 1: {
                    cell.textLabel.text= @"Acapella - Female 2";
                    onSwitch.on = !_loop2.channelIsMuted;
                    slider.value = _loop2.volume;
                    [onSwitch addTarget:self action:@selector(loop2SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop2VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                    
                case 2: {
                    cell.textLabel.text= @"Impulse";
                    onSwitch.on = !_loop1.channelIsMuted;
                    slider.value = _loop1.volume;
                    [onSwitch addTarget:self action:@selector(loop1SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop1VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                    
                case 3: {
                    cell.textLabel.text= @"Acapella Female";
                    onSwitch.on = !_loop4.channelIsMuted;
                    slider.value = _loop4.volume;
                    [onSwitch addTarget:self action:@selector(loop4SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop4VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
            }
            break;
        }
             
        case 2: {
            //Problem: need to set sliders outside of these functions so that they dont get reset when the table is scrolled up and down
            cell.accessoryView = [[UISwitch alloc] initWithFrame:CGRectZero];
            
            switch ( indexPath.row ) {
                case 0: {
                    cell.textLabel.text = @"AutoSoundMove";
                    ((UISwitch*)cell.accessoryView).on = _limiter != nil;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(limiterSwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 1: {
                    cell.textLabel.text = @"Reverb";
                    ((UISwitch*)cell.accessoryView).on = _reverbBlock != nil;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(reverbSwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 2: {
                    cell.textLabel.text = @"Room Size";
                    UISlider *roomSizeSlider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    roomSizeSlider.minimumValue = 0.1;
                    roomSizeSlider.maximumValue = 1.0;
                    roomSizeSlider.value = rSize;
                    [roomSizeSlider setContinuous:NO];
                    [roomSizeSlider addTarget:self action:@selector(roomSizeChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = roomSizeSlider;
                    break;
                }
                case 3:{
                    cell.textLabel.text= @"RT60 Value";
                    UISlider *rt60Slider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    rt60Slider.minimumValue = 0.2f;
                    rt60Slider.maximumValue = 3.0f;
                    rt60Slider.value = rt60Val;
                    [rt60Slider setContinuous:NO];
                    [rt60Slider addTarget:self action:@selector(rt60SizeChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = rt60Slider;
                    break;

                }
                case 4:{
                    cell.textLabel.text= @"widthRatio %";
                    UISlider *directMix = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    directMix.minimumValue = 0.1f;
                    directMix.maximumValue = 0.9f;
                    directMix.value = wRatio;
                    [directMix setContinuous:NO];
                    [directMix addTarget:self action:@selector(widthRatioSizeChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = directMix;
                      break;
                }
                    
            }
            break;
        }
        case 3: {
            cell.accessoryView = [[UISwitch alloc] initWithFrame:CGRectZero];
            
            switch ( indexPath.row ) {
                case 0: {
                    cell.textLabel.text = @"Input Playthrough";
                    ((UISwitch*)cell.accessoryView).on = _playthrough != nil;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(playthroughSwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 1: {
                    cell.textLabel.text = @"Direct Portion On";
                    ((UISwitch*)cell.accessoryView).on = directOn;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(directPortionChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 2: {
                    cell.textLabel.text = @"Reverb Portion On";
                    
                    ((UISwitch*)cell.accessoryView).on = reverbOn;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(reverbPortionChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 3:{
                    cell.textLabel.text = @"RoomRayModel Portion On";
                    
                    ((UISwitch*)cell.accessoryView).on = true;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(roomRayModelChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 4:{
                    cell.textLabel.text = @"Lock Soundsource On";
                    
                    ((UISwitch*)cell.accessoryView).on = ssLock;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(soundSourceLock:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                
            }
            break;
        }
        case 4:
            [cell addSubview:Listener];
            [cell addSubview:SoundSource];
            break;
            
    }
    
    return cell;
}

- (void) listenerTouched:(UIButton*) sender withEvent:(UIEvent*) event{
    
    CGPoint point = [[[event allTouches]anyObject] locationInView:self.tableView];
    point = [sender.superview convertPoint:point fromView:self.tableView];

    if (point.x < 0){
        point.x = 0.f;
    }
    if (point.y < 0){
        point.y = 0.f;
    }
    if (point.x > self.tableView.bounds.size.width){
        point.x = self.tableView.bounds.size.width;
    }
    if (point.y > self.tableView.bounds.size.width){
        point.y = self.tableView.bounds.size.width;
    }
    sender.center = point;
        
    
    if (ssLock){
        CGPoint ssP = CGPointMake(point.x + xDiff, point.y + yDiff);
        if (ssP.y < 0.0f){
            ssP.y = 0.0f;
        }
        if (ssP.x < 0.0f){
            ssP.x = 0.0f;
        }
        if (ssP.x > self.tableView.bounds.size.width){
            ssP.x = self.tableView.bounds.size.width;
        }
        if (ssP.y > self.tableView.bounds.size.width){
            ssP.y = self.tableView.bounds.size.width;
        }
        SoundSource.center = ssP;
    }
        
    
}

- (void) listenerTouchedEnds:(UIButton*) sender withEvent:(UIEvent*) event{

    CGPoint p = sender.center;
    
    xLloc = p.x;
    yLloc = p.y;
    
    float x = p.x / self.tableView.bounds.size.width;
    float y = p.y / self.tableView.bounds.size.width;

    float loc[2] = {x,y};

   // printf("Listener location is now : %f %f ", loc[0], loc[1]);
    if (!ssLock){
    Reverb.setListenerLocation(loc);
    }
    
    else{
        CGPoint ssp = SoundSource.center;
        float x1 = ssp.x / self.tableView.bounds.size.width;
        float y1 = ssp.y / self.tableView.bounds.size.width;
        float loc1[2] = {x1,y1};
        Reverb.setSoundAndListenerLocation(loc, loc1);
    }
    

}

- (void) soundSourceTouchedEnds:(UIButton*) sender withEvent:(UIEvent*) event{

    if (!ssLock){
    CGPoint p = sender.center;
    
        xDiff = p.x - xLloc;
        yDiff = p.y - yLloc;
        
    float x = p.x / self.tableView.bounds.size.width;
    float y = p.y / self.tableView.bounds.size.width;
    
        
    float loc[2] = {x,y};
    
    
    Reverb.setSoundLocation(loc);
     //   printf("Sound location is now : %f %f \n", loc[0], loc[1]);

    }
    
    

}

- (void) soundSourceTouched:(UIButton*) sender withEvent:(UIEvent* )event{

    if (!ssLock){
    CGPoint point = [[[event allTouches]anyObject] locationInView:self.tableView];

    point = [sender.superview convertPoint:point fromView:self.tableView];
    if (point.x < 0){
        point.x = 0.f;
    }
    else if (point.x > self.tableView.bounds.size.width){
        point.x = self.tableView.bounds.size.width;
    }
    if (point.y < 0){
        point.y = 0.f;
    }
    else if (point.y > self.tableView.bounds.size.width){
        point.y = self.tableView.bounds.size.width;
    }
    sender.center = point;
    }
    
}
- (CGFloat) tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 4){
        return tableView.bounds.size.width;
    }
    else{
        return 40;
    }
}

- (void)playthroughSwitchChanged:(UISwitch*)sender {
    if ( sender.isOn ) {
        self.playthrough = [[AEPlaythroughChannel alloc] initWithAudioController:_audioController];
        [_audioController addInputReceiver:_playthrough];
        [_audioController addChannels:@[_playthrough]];
    } else {
        [_audioController removeChannels:@[_playthrough]];
        [_audioController removeInputReceiver:_playthrough];
        self.playthrough = nil;
    }
}

- (void)directPortionChanged:(UISwitch*)sender {
    Reverb.setDirectONOFF(sender.on);
    directOn = sender.on;
    
}

-(void) roomRayModelChanged:(UISwitch*)sender{
    Reverb.setRoomRayModel(sender.on);
}

-(void)soundSourceLock:(UISwitch*) sender{
    ssLock = sender.on;
}


- (void)reverbPortionChanged:(UISwitch*)sender {
    Reverb.setReverbONOFF(sender.on) ;
    reverbOn = sender.on;
    
    
}
- (void)measurementModeSwitchChanged:(UISwitch*)sender {
    _audioController.useMeasurementMode = sender.on;
    
}

-(void)inputGainSliderChanged:(UISlider*)slider {
    _audioController.inputGain = slider.value;
}

-(void)highpassFrequencyChanged:(UISlider*)slider {
    equalizer.set1stOrderHighPass(slider.value);
}

-(void)bell1GainChanged:(UISlider*)slider {
    equalizer.setBell(250.0, 125.0, slider.value, 1);
}

-(void)bell2GainChanged:(UISlider*)slider {
    equalizer.setBell(1000.0, 500.0, slider.value, 2);
}

-(void)bell3GainChanged:(UISlider*)slider {
    equalizer.setBell(2500.0, 1250.0, slider.value, 3);
}

-(void)lowpassFrequencyChanged:(UISlider*)slider {
    equalizer.setLowPass(slider.value);
}

-(void)roomSizeChanged:(UISlider*)slider {

    Reverb.setRoomSize(slider.value);
    printf("Room size is now : %f \n", slider.value);
    rSize = slider.value;

}

- (void) rt60SizeChanged: (UISlider*)slider{

    Reverb.setRT60(slider.value);
    printf("RT60 is now : %f \n", slider.value);
    rt60Val = slider.value;

}

- (void) widthRatioSizeChanged: (UISlider*) slider{

    Reverb.setWidthRatio(slider.value);
    printf("Width Ratio is now : %f \n", slider.value);
    wRatio = slider.value;

}


- (void)limiterSwitchChanged:(UISwitch*)sender {
    if (sender.isOn){
        autoSoundMove = true;
    }
    else{
        autoSoundMove = false;
    }
}

- (void)expanderSwitchChanged:(UISwitch*)sender {
    if ( sender.isOn ) {
        self.expander = [[AEExpanderFilter alloc] init];
        [_audioController addFilter:_expander];
    } else {
        [_audioController removeFilter:_expander];
        self.expander = nil;
    }
}

- (void)calibrateExpander:(UIButton*)sender {
    if ( !_expander ) return;
    sender.enabled = NO;
    [_expander startCalibratingWithCompletionBlock:^{
        sender.enabled = YES;
    }];
}


- (void)reverbSwitchChanged:(UISwitch*)sender {
    if ( sender.isOn ) {
        self.reverbBlock = [AEBlockFilter filterWithBlock:^(AEAudioControllerFilterProducer producer,
                                                       void *producerToken,
                                                       const AudioTimeStamp *time,
                                                       UInt32 frames,
                                                       AudioBufferList *audio) {

            // Pull audio
            OSStatus status = producer(producerToken, audio, &frames);
            if ( status != noErr ) return;
            
            float * buff =(float*)audio->mBuffers[0].mData;
            if (!isnan(buff[0]) ){
            
            Reverb.processIFretlessBuffer((float*)audio->mBuffers[0].mData, frames, (float*)audio->mBuffers[0].mData  ,(float*) audio->mBuffers[1].mData);
            }
            else{
                printf("buffer is nan");
            }
        }];

        
        [_audioController addFilter:_reverbBlock];

    } else {
        [_audioController removeFilter:_reverbBlock];

        self.reverbBlock = nil;
    }
}

- (void)channelButtonPressed:(UIButton*)sender {
    BOOL selected = [_audioController.inputChannelSelection containsObject:@(sender.tag)];
    selected = !selected;
    if ( selected ) {
        _audioController.inputChannelSelection = [[_audioController.inputChannelSelection arrayByAddingObject:@(sender.tag)] sortedArrayUsingSelector:@selector(compare:)];
        [self performSelector:@selector(highlightButtonDelayed:) withObject:sender afterDelay:0.01];
    } else {
        NSMutableArray *channels = [_audioController.inputChannelSelection mutableCopy];
        [channels removeObject:@(sender.tag)];
        _audioController.inputChannelSelection = channels;
        sender.highlighted = NO;
    }
}

- (void)highlightButtonDelayed:(UIButton*)button {
    button.highlighted = YES;
}

static inline float translate(float val, float min, float max) {
    if ( val < min ) val = min;
    if ( val > max ) val = max;
    return (val - min) / (max - min);
}

float soundSourceLoc[10][2] = {{0.2, 0.9},{0.4,1.0},{0.9, 0.2},{0.3,0.3},{0.7,0.7},{0.1,0.8},{0,0},{1,1},{0.8,0.4},{0.3,0.9}};

int indexSoundSource = 0;



- (void) moveSoundSource: (NSTimer*) timer{
    if (autoSoundMove ){
//        counter = 0;
// 
//        CGFloat x = Listener.frame.origin.x + sin(angle) * 300.f;
//        if (x > self.tableView.bounds.size.width){
//            x =self.tableView.bounds.size.width;
//        }
//        CGFloat y = Listener.frame.origin.y + cos(angle) * 300.f;
//        if ( y >  self.tableView.bounds.size.width){
//            y =  self.tableView.bounds.size.width;
//        }
//        
//        if (x < 0){
//            x = 0.0f;
//        }
//        if (y < 0){
//            y = 0.0f;
//        }
        
        float x = 1.f-soundSourceLoc[indexSoundSource][0];
        float y = 1.f-soundSourceLoc[indexSoundSource][1];
        float xproc = x * self.tableView.bounds.size.width;
        float yproc = y * self.tableView.bounds.size.width;
//        float xf = x / self.tableView.bounds.size.width;
//        float yf = y / self.tableView.bounds.size.width;
        float loc[2] = {x,y};
        CGPoint p = CGPointMake(xproc, yproc);
        SoundSource.center = p;
        
        Reverb.setSoundLocation(loc);
        
        indexSoundSource++;
        if (indexSoundSource > 9){
            indexSoundSource = 0;
        }
    }
    
//    if (angle > 2.f * M_PI){
//        angle -= 2.f*M_PI;
//    }
    
    counter ++;
//    angle += 0.015f;
}

- (void)updateLevels:(NSTimer*)timer {
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    
    Float32 inputAvg, inputPeak, outputAvg, outputPeak;
    [_audioController inputAveragePowerLevel:&inputAvg peakHoldLevel:&inputPeak];
    [_audioController outputAveragePowerLevel:&outputAvg peakHoldLevel:&outputPeak];
    UIView *headerView = self.tableView.tableHeaderView;
    
    _inputLevelLayer.frame = CGRectMake(headerView.bounds.size.width/2.0 - 5.0 - (translate(inputAvg, -20, 0) * (headerView.bounds.size.width/2.0 - 15.0)),
                                        90,
                                        translate(inputAvg, -20, 0) * (headerView.bounds.size.width/2.0 - 15.0),
                                        10);
    
    _outputLevelLayer.frame = CGRectMake(headerView.bounds.size.width/2.0,
                                         _outputLevelLayer.frame.origin.y, 
                                         translate(outputAvg, -20, 0) * (headerView.bounds.size.width/2.0 - 15.0),
                                         10);
    
    [CATransaction commit];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if ( context == &kInputChannelsChangedContext ) {
        [self.tableView reloadSections:[NSIndexSet indexSetWithIndex:3] withRowAnimation:UITableViewRowAnimationFade];
    }
}




@end
