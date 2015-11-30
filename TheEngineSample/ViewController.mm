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
#import "AERecorder.h"
#import <QuartzCore/QuartzCore.h>
#import "FDN.h"
#import "MultiLevelBiQuadFilter.h"
#import "DoubleBufferedReverb.h"

static int kInputChannelsChangedContext;

#define kAuxiliaryViewTag 251


@interface ViewController () {
    AudioFileID _audioUnitFile;
    AEChannelGroupRef _group;
//    FDN reverb;
//    FDN reverb2;
//    FDN* reverbPointer;
//    FDN* reverb2Pointer;
    DoubleBufferedReverb Reverb;
    MultiLevelBiQuadFilter equalizer;
    UIButton *Listener;
    UIButton *SoundSource;
    bool autoSoundMove;
    float angle;
    int counter;


}
@property (nonatomic, strong) AEAudioController *audioController;
@property (nonatomic, strong) AEAudioFilePlayer *loop1;
@property (nonatomic, strong) AEAudioFilePlayer *loop2;
@property (nonatomic, strong) AEAudioFilePlayer *loop3;
@property (nonatomic, strong) AEAudioFilePlayer *loop4;
@property (nonatomic, strong) AEBlockChannel *oscillator;
@property (nonatomic, strong) AEAudioUnitChannel *audioUnitPlayer;
//@property (nonatomic, strong) AEAudioFilePlayer *oneshot;
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

//@property (nonatomic, strong) AERecorder *recorder;
//@property (nonatomic, strong) AEAudioFilePlayer *player;
//@property (nonatomic, strong) UIButton *recordButton;
//@property (nonatomic, strong) UIButton *playButton;
//@property (nonatomic, strong) UIButton *oneshotButton;
//@property (nonatomic, strong) UIButton *oneshotAudioUnitButton;
@end

@implementation ViewController


- (id)initWithAudioController:(AEAudioController*)audioController {
    if ( !(self = [super initWithStyle:UITableViewStyleGrouped]) ) return nil;
    
    counter = 0;
    self.number = 1.0f;
//    reverbPointer = &reverb;
//    reverb2Pointer = &reverb2;
    
    
    
    autoSoundMove = false;
    angle = 0.0f;
        self.soundSourceTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(moveSoundSource:) userInfo:nil repeats:YES];
    
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
    
    

    //Try: crowd, acapella, allofthestars, snap, button
    
    self.loop1 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"snap" withExtension:@"wav"] error:NULL];
    _loop1.volume = 1.0;
    _loop1.channelIsMuted = YES;
    _loop1.loop = YES;
    
    self.loop2 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"design2" withExtension:@"wav"] error:NULL];
    _loop2.volume = 1.0;
    _loop2.channelIsMuted = YES;
    _loop2.loop = YES;
    
    // Create the third loop player
    self.loop3 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"hello" withExtension:@"wav"] error:NULL];
    _loop3.volume = 1.0;
    _loop3.channelIsMuted = YES;
    _loop3.loop = YES;
    
    
    self.loop4 = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"someonelikeyou" withExtension:@"wav"] error:NULL];
    _loop4.volume = 1.0;
    _loop4.channelIsMuted = YES;
    _loop4.loop = YES;
    

    
    
    // initialise the reverb
    //reverb.setRoomSize(5.0f);
   // reverb.setTone(1.0f);
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

    
    // Create an audio unit channel (a file player)
   // self.audioUnitPlayer = [[AEAudioUnitChannel alloc] initWithComponentDescription:AEAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Generator, kAudioUnitSubType_AudioFilePlayer)];
    
    // Create a group for loop1, loop2 and oscillator
    
    _group = [_audioController createChannelGroup];
    [_audioController addChannels:@[ _loop1,_loop2,_loop3, _loop4, _oscillator] toChannelGroup:_group];
     
    // Finally, add the audio unit player
   // [_audioController addChannels:@[_audioUnitPlayer]];
    
    
    [_audioController addObserver:self forKeyPath:@"numberOfInputChannels" options:0 context:(void*)&kInputChannelsChangedContext];
    _audioController.inputGain = 1.0f;
    return self;
}

-(void)dealloc {
    [_audioController removeObserver:self forKeyPath:@"numberOfInputChannels"];
    
    if ( _levelsTimer ) [_levelsTimer invalidate];

    NSMutableArray *channelsToRemove = [NSMutableArray arrayWithObjects:nil];

/*
    if ( _player ) {
        [channelsToRemove addObject:_player];
    }
    
    if ( _oneshot ) {
        [channelsToRemove addObject:_oneshot];
    }
 */
    
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
    
    
//    UIScrollView *scrollView = [[UIScrollView alloc] initWithFrame:CGRectMake(0, 0, self.tableView.bounds.size.width, self.tableView.bounds.size.height)]; // this makes the scroll view - set the frame as the size you want to SHOW on the screen
//    [scrollView setContentSize:CGSizeMake(self.tableView.bounds.size.width , self.tableView.bounds.size.height)];
//    scrollView.backgroundColor = [UIColor redColor];
//    
//    /* you can do this bit as many times as you want... make sure you set each image at a different origin */
//    UIImageView *imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"/*IMAGE*/"]]; // this makes the image view
//    [scrollView addSubview:imageView]; // this adds the image to the scrollview
//    /* end adding image */
//    
//
//    
//    UIImageView *image = [[UIImageView alloc] initWithFrame:CGRectMake(0,0, 300, 5000)];
//    image.backgroundColor = [UIColor blueColor];
//    [scrollView addSubview:image];
//    
//    [self.view addSubview:scrollView];
//
//    UIView *footerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.tableView.bounds.size.width, 80)];
//    self.recordButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
//    [_recordButton setTitle:@"Record" forState:UIControlStateNormal];
//    [_recordButton setTitle:@"Stop" forState:UIControlStateSelected];
//    [_recordButton addTarget:self action:@selector(record:) forControlEvents:UIControlEventTouchUpInside];
//    _recordButton.frame = CGRectMake(20, 10, ((footerView.bounds.size.width-50) / 2), footerView.bounds.size.height - 20);
//    _recordButton.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleRightMargin;
//    self.playButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
//    [_playButton setTitle:@"Play" forState:UIControlStateNormal];
//    [_playButton setTitle:@"Stop" forState:UIControlStateSelected];
//    [_playButton addTarget:self action:@selector(play:) forControlEvents:UIControlEventTouchUpInside];
//    _playButton.frame = CGRectMake(CGRectGetMaxX(_recordButton.frame)+10, 10, ((footerView.bounds.size.width-50) / 2), footerView.bounds.size.height - 20);
//    _playButton.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleLeftMargin;
//    [footerView addSubview:_recordButton];
//    [footerView addSubview:_playButton];
//    self.tableView.tableFooterView = footerView;
//     
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
        case 0:
            return 3;
            
        case 1:
            return 4;
            
        case 2:
            return 6;
            
        case 3:
            return 3 + (_audioController.numberOfInputChannels > 1 ? 1 : 0);
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

//UITableViewCell *cellFour;

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
            /*
            cell.accessoryView = [[UISwitch alloc] initWithFrame:CGRectZero];
            UISlider *slider = [[UISlider alloc] initWithFrame:CGRectMake(cell.bounds.size.width - (isiPad ? 250 : 210), 0, 100, cell.bounds.size.height)];
            slider.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
            slider.tag = kAuxiliaryViewTag;
            slider.maximumValue = 1.0;
            slider.minimumValue = 0.0;
            [cell addSubview:slider];
             */
            
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
                    cell.textLabel.text= @"Acapella - Male";
                    onSwitch.on = !_loop3.channelIsMuted;
                    slider.value = _loop3.volume;
                    [onSwitch addTarget:self action:@selector(loop3SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop3VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 1: {
                    cell.textLabel.text= @"Poem - Female";
                    onSwitch.on = !_loop2.channelIsMuted;
                    slider.value = _loop2.volume;
                    [onSwitch addTarget:self action:@selector(loop2SwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [slider addTarget:self action:@selector(loop2VolumeChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                    
                case 2: {
                    cell.textLabel.text= @"Snapping Fingers";
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
                    cell.textLabel.text = @"Expander";
                    UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 250, 40)];
                    UIButton *calibrateButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
                    calibrateButton.translatesAutoresizingMaskIntoConstraints = NO;
                    [calibrateButton setTitle:@"Calibrate" forState:UIControlStateNormal];
                    [calibrateButton addTarget:self action:@selector(calibrateExpander:) forControlEvents:UIControlEventTouchUpInside];
                    UISwitch * onSwitch = [[UISwitch alloc] initWithFrame:CGRectZero];
                    onSwitch.translatesAutoresizingMaskIntoConstraints = NO;
                    onSwitch.on = _expander != nil;
                    [onSwitch addTarget:self action:@selector(expanderSwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    [view addSubview:calibrateButton];
                    [view addSubview:onSwitch];
                    [view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[calibrateButton][onSwitch]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(calibrateButton, onSwitch)]];
                    [view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[calibrateButton]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(calibrateButton)]];
                    [view addConstraint:[NSLayoutConstraint constraintWithItem:onSwitch attribute:NSLayoutAttributeCenterY relatedBy:NSLayoutRelationEqual toItem:view attribute:NSLayoutAttributeCenterY multiplier:1 constant:0]];
                    cell.accessoryView = view;
                    break;
                }
                case 2: {
                    cell.textLabel.text = @"Reverb";
                    ((UISwitch*)cell.accessoryView).on = _reverbBlock != nil;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(reverbSwitchChanged:) forControlEvents:UIControlEventValueChanged];
                    break;
                }
                case 3: {
                    cell.textLabel.text = @"Room Size";
                    UISlider *roomSizeSlider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    roomSizeSlider.minimumValue = 0.1;
                    roomSizeSlider.maximumValue = 1.0;
                    roomSizeSlider.value = 0.15;
                    [roomSizeSlider setContinuous:NO];
                    [roomSizeSlider addTarget:self action:@selector(roomSizeChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = roomSizeSlider;
                    break;
                }
                case 4:{
                    cell.textLabel.text= @"RT60 Value";
                    UISlider *rt60Slider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    rt60Slider.minimumValue = 0.2f;
                    rt60Slider.maximumValue = 3.0f;
                    rt60Slider.value = 0.7f;
                    [rt60Slider setContinuous:NO];
                    [rt60Slider addTarget:self action:@selector(rt60SizeChanged:) forControlEvents:UIControlEventValueChanged];
                    cell.accessoryView = rt60Slider;
                    break;

                }
                case 5:{
                    cell.textLabel.text= @"widthRatio %";
                    UISlider *directMix = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 300, 40)];
                    directMix.minimumValue = 0.1f;
                    directMix.maximumValue = 0.9f;
                    directMix.value = 0.5f;
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
                    cell.textLabel.text = @"Measurement Mode";
                    ((UISwitch*)cell.accessoryView).on = _audioController.useMeasurementMode;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(measurementModeSwitchChanged:) forControlEvents:UIControlEventValueChanged];

                    break;
                }
                case 2: {
                    cell.textLabel.text = @"Direct Portion On";
                    ((UISwitch*)cell.accessoryView).on = true;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(directPortionChanged:) forControlEvents:UIControlEventValueChanged];
//                    UISlider *inputGainSlider = [[UISlider alloc] initWithFrame:CGRectMake(0, 0, 100, 40)];
//                    inputGainSlider.minimumValue = 0.0;
//                    inputGainSlider.maximumValue = 1.0;
//                    inputGainSlider.value = _audioController.inputGain;
//                    [inputGainSlider addTarget:self action:@selector(inputGainSliderChanged:) forControlEvents:UIControlEventValueChanged];
          //          cell.accessoryView = inputGainSlider;
                    break;
                }
                case 3: {
                    cell.textLabel.text = @"Reverb Portion On";
                    
                    ((UISwitch*)cell.accessoryView).on = true;
                    [((UISwitch*)cell.accessoryView) addTarget:self action:@selector(reverbPortionChanged:) forControlEvents:UIControlEventValueChanged];
//                    int channelCount = _audioController.numberOfInputChannels;
//                    CGSize buttonSize = CGSizeMake(30, 30);
//
//                    UIScrollView *channelStrip = [[UIScrollView alloc] initWithFrame:CGRectMake(0,
//                                                                                                 0,
//                                                                                                 MIN(channelCount * (buttonSize.width+5) + 5,
//                                                                                                     isiPad ? 400 : 200),
//                                                                                                 cell.bounds.size.height)];
//                    channelStrip.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
//                    channelStrip.backgroundColor = [UIColor clearColor];
//                    
//                    for ( int i=0; i<channelCount; i++ ) {
//                        UIButton *button = [UIButton buttonWithType:UIButtonTypeRoundedRect];
//                        button.frame = CGRectMake(i*(buttonSize.width+5), round((channelStrip.bounds.size.height-buttonSize.height)/2), buttonSize.width, buttonSize.height);
//                        [button setTitle:[NSString stringWithFormat:@"%d", i+1] forState:UIControlStateNormal];
//                        button.highlighted = [_audioController.inputChannelSelection containsObject:@(i)];
//                        button.tag = i;
//                        [button addTarget:self action:@selector(channelButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
//                        [channelStrip addSubview:button];
//                    }
//                    
//                    channelStrip.contentSize = CGSizeMake(channelCount * (buttonSize.width+5) + 5, channelStrip.bounds.size.height);
//                    
//                    cell.accessoryView = channelStrip;
               
                    break;
                }
            }
            break;
        }
        case 4:
//            UIButton *Listener = [UIButton buttonWithType:UIButtonTypeCustom];
//            [Listener addTarget:self action:@selector(listenerTouched:withEvent:) forControlEvents:UIControlEventTouchDragInside|UIControlEventTouchDragOutside];
//            [Listener addTarget:self action:@selector(listenerTouchedEnds:withEvent:) forControlEvents:UIControlEventTouchUpInside | UIControlEventTouchUpOutside];
//            [Listener setFrame:CGRectMake(self.tableView.bounds.size.width/2-25, 2*self.tableView.bounds.size.width/3, 50, 50)];
//            UIImage *listenerImage = [UIImage imageNamed:@"person.png"];
//            [Listener setImage:listenerImage forState:UIControlStateNormal];
//           
            [cell addSubview:Listener];
            
//            UIButton *SoundSource = [UIButton buttonWithType:UIButtonTypeCustom];
//            [SoundSource addTarget:self action:@selector(soundSourceTouched:withEvent:) forControlEvents:UIControlEventTouchDragInside| UIControlEventTouchDragOutside];
//            [SoundSource addTarget:self action:@selector(soundSourceTouchedEnds:withEvent:) forControlEvents:UIControlEventTouchUpOutside | UIControlEventTouchUpInside];
//            
//            [SoundSource setFrame:CGRectMake(self.tableView.bounds.size.width/2-25, self.tableView.bounds.size.width/3, 50, 50)];
//            UIImage *soundSourceImage = [UIImage imageNamed:@"Speaker.png"];
//            UIImage *soundSourceImageTouched = [UIImage imageNamed:@"SpeakerTouched.png"];
//            [SoundSource setImage:soundSourceImage  forState:UIControlStateNormal];
//            [SoundSource setImage:soundSourceImageTouched forState:UIControlStateHighlighted];
//            
         //   UIImage* cellImage = [UIImage imageNamed:@"tiles.jpg"];
         //   [cell setContentMode:UIViewContentModeScaleToFill];
         //   [cell setImage:cellImage];
            
          //  SoundSource.backgroundColor = [UIColor blueColor];
            [cell addSubview:SoundSource];
          //  cellFour = cell;
            
            break;
            
    }
    
    return cell;
}

- (void) listenerTouched:(UIButton*) sender withEvent:(UIEvent*) event{
    //  printf("DRAGGED");
    
    CGPoint point = [[[event allTouches]anyObject] locationInView:self.tableView];
    //  printf("Location x is: %f, location y is %f \n", point.x, point.y);
    //  printf("sender center was : %f, %f\n", sender.center.x, sender.center.y);
    
    point = [sender.superview convertPoint:point fromView:self.tableView];
    //  printf("New point is : %f , %f, \n", point.x, point.y);
    if (point.x < 0){
        point.x = 0;
    }
    if (point.y < 0){
        point.y = 0;
    }
    sender.center = point;
}

- (void) listenerTouchedEnds:(UIButton*) sender withEvent:(UIEvent*) event{
    //Update the final position here
  //  printf("touch ends");
    CGPoint p = sender.center;
    float x = p.x / self.tableView.bounds.size.width;
    float y = p.y / self.tableView.bounds.size.width;
  //  printf("X loc is %f, y loc is %f \n", x, y);
    float loc[2] = {x,y};
    
//    [self flip];
    
    Reverb.setListenerLocation(loc);

 
//    self.changeListenerTimer = [NSTimer scheduledTimerWithTimeInterval:0.2 target:self selector:@selector(swapListener:) userInfo:nil repeats:NO];
    

}

//- (void) flip{
//    printf("Method swap \n");
//    FDN* temp = reverbPointer;
//    reverbPointer = reverb2Pointer;
//    reverb2Pointer = temp;
//}
//
//
//-(void) swapListener: (NSTimer*) timer{
//    printf("Timer Swap \n");
//    FDN* temp = reverbPointer;
//    reverbPointer = reverb2Pointer;
//    reverb2Pointer = temp;
//    reverb2Pointer->setListenerLoc(updateloc);
//    
//}
//
//-(void) swapSoundSource: (NSTimer*) timer{
//    printf("Timer Swap \n");
//    FDN* temp = reverbPointer;
//    reverbPointer = reverb2Pointer;
//    reverb2Pointer = temp;
//    reverb2Pointer->setSoundSourceLoc(updateloc);
//    
//}


- (void) soundSourceTouchedEnds:(UIButton*) sender withEvent:(UIEvent*) event{
    //Update the final position here
  //  printf("touch ends");
    CGPoint p = sender.center;
    float x = p.x / self.tableView.bounds.size.width;
    float y = p.y / self.tableView.bounds.size.width;
 //   printf("X loc is %f, y loc is %f \n", x, y);
    float loc[2] = {x,y};
    
    Reverb.setSoundLocation(loc);
//    [self flip];
//    reverb2Pointer->setSoundSourceLoc(loc);
//    
//    self.changeListenerTimer = [NSTimer scheduledTimerWithTimeInterval:0.2 target:self selector:@selector(swapSoundSource:) userInfo:nil repeats:NO];

}

- (void) soundSourceTouched:(UIButton*) sender withEvent:(UIEvent* )event{
  //  printf("DRAGGED");
    
    CGPoint point = [[[event allTouches]anyObject] locationInView:self.tableView];
  //  printf("Location x is: %f, location y is %f \n", point.x, point.y);
  //  printf("sender center was : %f, %f\n", sender.center.x, sender.center.y);

    point = [sender.superview convertPoint:point fromView:self.tableView];
  //  printf("New point is : %f , %f, \n", point.x, point.y);
    if (point.x < 0){
        point.x = 0;
    }
    else if (point.x > self.tableView.bounds.size.width){
        point.x = self.tableView.bounds.size.width;
    }
    if (point.y < 0){
        point.y = 0;
    }
    else if (point.y > self.tableView.bounds.size.width){
        point.y = self.tableView.bounds.size.width;
    }
    sender.center = point;
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
/*
- (void)loop1SwitchChanged:(UISwitch*)sender {
    _loop1.channelIsMuted = !sender.isOn;
}

- (void)loop1VolumeChanged:(UISlider*)sender {
    _loop1.volume = sender.value;
}

- (void)loop2SwitchChanged:(UISwitch*)sender {
    _loop2.channelIsMuted = !sender.isOn;
}

- (void)loop2VolumeChanged:(UISlider*)sender {
    _loop2.volume = sender.value;
}

- (void)oscillatorSwitchChanged:(UISwitch*)sender {
    _oscillator.channelIsMuted = !sender.isOn;
}

- (void)oscillatorVolumeChanged:(UISlider*)sender {
    _oscillator.volume = sender.value;
}

- (void)channelGroupSwitchChanged:(UISwitch*)sender {
    [_audioController setMuted:!sender.isOn forChannelGroup:_group];
}

- (void)channelGroupVolumeChanged:(UISlider*)sender {
    [_audioController setVolume:sender.value forChannelGroup:_group];
}

- (void)oneshotPlayButtonPressed:(UIButton*)sender {
    if ( _oneshot ) {
        [_audioController removeChannels:@[_oneshot]];
        self.oneshot = nil;
        _oneshotButton.selected = NO;
    } else {
        self.oneshot = [AEAudioFilePlayer audioFilePlayerWithURL:[[NSBundle mainBundle] URLForResource:@"Organ Run" withExtension:@"m4a"] error:NULL];
        _oneshot.removeUponFinish = YES;
        __weak ViewController *weakSelf = self;
        _oneshot.completionBlock = ^{
            ViewController *strongSelf = weakSelf;
            strongSelf.oneshot = nil;
            strongSelf->_oneshotButton.selected = NO;
        };
        [_audioController addChannels:@[_oneshot]];
        _oneshotButton.selected = YES;
    }
}
 */

/*
- (void)oneshotAudioUnitPlayButtonPressed:(UIButton*)sender {
    if ( !_audioUnitFile ) {
        NSURL *playerFile = [[NSBundle mainBundle] URLForResource:@"Organ Run" withExtension:@"m4a"];
        AECheckOSStatus(AudioFileOpenURL((__bridge CFURLRef)playerFile, kAudioFileReadPermission, 0, &_audioUnitFile), "AudioFileOpenURL");
    }
    
    // Set the file to play
    AECheckOSStatus(AudioUnitSetProperty(_audioUnitPlayer.audioUnit, kAudioUnitProperty_ScheduledFileIDs, kAudioUnitScope_Global, 0, &_audioUnitFile, sizeof(_audioUnitFile)),
                "AudioUnitSetProperty(kAudioUnitProperty_ScheduledFileIDs)");

    // Determine file properties
    UInt64 packetCount;
	UInt32 size = sizeof(packetCount);
	AECheckOSStatus(AudioFileGetProperty(_audioUnitFile, kAudioFilePropertyAudioDataPacketCount, &size, &packetCount),
                "AudioFileGetProperty(kAudioFilePropertyAudioDataPacketCount)");
	
	AudioStreamBasicDescription dataFormat;
	size = sizeof(dataFormat);
	AECheckOSStatus(AudioFileGetProperty(_audioUnitFile, kAudioFilePropertyDataFormat, &size, &dataFormat),
                "AudioFileGetProperty(kAudioFilePropertyDataFormat)");
    
	// Assign the region to play
	ScheduledAudioFileRegion region;
	memset (&region.mTimeStamp, 0, sizeof(region.mTimeStamp));
	region.mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
	region.mTimeStamp.mSampleTime = 0;
	region.mCompletionProc = NULL;
	region.mCompletionProcUserData = NULL;
	region.mAudioFile = _audioUnitFile;
	region.mLoopCount = 0;
	region.mStartFrame = 0;
	region.mFramesToPlay = (UInt32)packetCount * dataFormat.mFramesPerPacket;
	AECheckOSStatus(AudioUnitSetProperty(_audioUnitPlayer.audioUnit, kAudioUnitProperty_ScheduledFileRegion, kAudioUnitScope_Global, 0, &region, sizeof(region)),
                "AudioUnitSetProperty(kAudioUnitProperty_ScheduledFileRegion)");
	
	// Prime the player by reading some frames from disk
	UInt32 defaultNumberOfFrames = 0;
	AECheckOSStatus(AudioUnitSetProperty(_audioUnitPlayer.audioUnit, kAudioUnitProperty_ScheduledFilePrime, kAudioUnitScope_Global, 0, &defaultNumberOfFrames, sizeof(defaultNumberOfFrames)),
                "AudioUnitSetProperty(kAudioUnitProperty_ScheduledFilePrime)");
    
    // Set the start time (now = -1)
    AudioTimeStamp startTime;
	memset (&startTime, 0, sizeof(startTime));
	startTime.mFlags = kAudioTimeStampSampleTimeValid;
	startTime.mSampleTime = -1;
	AECheckOSStatus(AudioUnitSetProperty(_audioUnitPlayer.audioUnit, kAudioUnitProperty_ScheduleStartTimeStamp, kAudioUnitScope_Global, 0, &startTime, sizeof(startTime)),
			   "AudioUnitSetProperty(kAudioUnitProperty_ScheduleStartTimeStamp)");

}
 */

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
    
}

- (void)reverbPortionChanged:(UISwitch*)sender {
    Reverb.setReverbONOFF(sender.on) ;
    
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
//    reverb.setRoomSize(slider.value);
//     reverb2.setRoomSize(slider.value);
}

- (void) rt60SizeChanged: (UISlider*)slider{
    Reverb.setRT60(slider.value);
//    reverb.setRT60(slider.value);
//    reverb2.setRT60(slider.value);
}

- (void) widthRatioSizeChanged: (UISlider*) slider{
//    reverb.setDirectMix(slider.value);
    Reverb.setWidthRatio(slider.value);
//    reverb.setWidthRatio(slider.value);
//    reverb2.setWidthRatio(slider.value);
    
}


- (void)limiterSwitchChanged:(UISwitch*)sender {
    if (sender.isOn){
        autoSoundMove = true;
    }
    else{
        autoSoundMove = false;
    }
//    if ( sender.isOn ) {
//        self.limiter = [[AELimiterFilter alloc] init];
//        _limiter.level = 0.1;
//        [_audioController addFilter:_limiter];
//    } else {
//        [_audioController removeFilter:_limiter];
//        self.limiter = nil;
//    }
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
        /*
        self.reverb = [[AEAudioUnitFilter alloc] initWithComponentDescription:AEAudioComponentDescriptionMake(kAudioUnitManufacturer_Apple, kAudioUnitType_Effect, kAudioUnitSubType_Reverb2) preInitializeBlock:^(AudioUnit audioUnit) {
            AudioUnitSetParameter(audioUnit, kReverb2Param_DryWetMix, kAudioUnitScope_Global, 0, 100.f, 0);
        }];
        */
        
        self.reverbBlock = [AEBlockFilter filterWithBlock:^(AEAudioControllerFilterProducer producer,
                                                       void *producerToken,
                                                       const AudioTimeStamp *time,
                                                       UInt32 frames,
                                                       AudioBufferList *audio) {

            // Pull audio
            OSStatus status = producer(producerToken, audio, &frames);
            if ( status != noErr ) return;
            
            
            // run equalizer on left channel, out to buffer
            equalizer.processBuffer((float*)audio->mBuffers[0].mData, (float*)audio->mBuffers[0].mData, frames);

            
            Reverb.processIFretlessBuffer((float*)audio->mBuffers[0].mData, frames, (float*)audio->mBuffers[0].mData  ,(float*) audio->mBuffers[1].mData);

            
//            printf("left2 : %f right2: %f \n", left2[0], right2[0]);
//            audio->mBuffers[0].mData = left2;
//            audio->mBuffers[1].mData = right2;


            
            
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

/*
- (void)record:(id)sender {
    if ( _recorder ) {
        [_recorder finishRecording];
        [_audioController removeOutputReceiver:_recorder];
        [_audioController removeInputReceiver:_recorder];
        self.recorder = nil;
        _recordButton.selected = NO;
    } else {
        self.recorder = [[AERecorder alloc] initWithAudioController:_audioController];
        NSArray *documentsFolders = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *path = [documentsFolders[0] stringByAppendingPathComponent:@"Recording.m4a"];
        NSError *error = nil;
        if ( ![_recorder beginRecordingToFileAtPath:path fileType:kAudioFileM4AType error:&error] ) {
            [[[UIAlertView alloc] initWithTitle:@"Error" 
                                         message:[NSString stringWithFormat:@"Couldn't start recording: %@", [error localizedDescription]]
                                        delegate:nil
                               cancelButtonTitle:nil
                               otherButtonTitles:@"OK", nil] show];
            self.recorder = nil;
            return;
        }
        
        _recordButton.selected = YES;
        
        [_audioController addOutputReceiver:_recorder];
        [_audioController addInputReceiver:_recorder];
    }
}

- (void)play:(id)sender {
    if ( _player ) {
        [_audioController removeChannels:@[_player]];
        self.player = nil;
        _playButton.selected = NO;
    } else {
        NSArray *documentsFolders = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *path = [documentsFolders[0] stringByAppendingPathComponent:@"Recording.m4a"];
        
        if ( ![[NSFileManager defaultManager] fileExistsAtPath:path] ) return;
        
        NSError *error = nil;
        self.player = [AEAudioFilePlayer audioFilePlayerWithURL:[NSURL fileURLWithPath:path] error:&error];
        
        if ( !_player ) {
            [[[UIAlertView alloc] initWithTitle:@"Error" 
                                         message:[NSString stringWithFormat:@"Couldn't start playback: %@", [error localizedDescription]]
                                        delegate:nil
                               cancelButtonTitle:nil
                               otherButtonTitles:@"OK", nil] show];
            return;
        }
        
        _player.removeUponFinish = YES;
        __weak ViewController *weakSelf = self;
        _player.completionBlock = ^{
            ViewController *strongSelf = weakSelf;
            strongSelf->_playButton.selected = NO;
            weakSelf.player = nil;
        };
        [_audioController addChannels:@[_player]];
        
        _playButton.selected = YES;
    }
}
 */

static inline float translate(float val, float min, float max) {
    if ( val < min ) val = min;
    if ( val > max ) val = max;
    return (val - min) / (max - min);
}


- (void) moveSoundSource: (NSTimer*) timer{
    if (autoSoundMove && counter % 15 == 0){
        counter = 0;
      //  printf("x y c %f %f %d\n", SoundSource.frame.origin.x, SoundSource.frame.origin.y, counter );
 
        CGFloat x = Listener.frame.origin.x + sin(angle) * 300;
        if (x > self.tableView.bounds.size.width){
            x =self.tableView.bounds.size.width;
        }
        CGFloat y = Listener.frame.origin.y + cos(angle) * 300;
        if ( y >  self.tableView.bounds.size.width){
            y =  self.tableView.bounds.size.width;
        }
        
        
        float xf = x / self.tableView.bounds.size.width;
        float yf = y / self.tableView.bounds.size.width;
      //  printf("X loc is %f, y loc is %f \n", x, y);
        float loc[2] = {xf,yf};
        CGPoint p = CGPointMake(x, y);
        SoundSource.center = p;
        
        Reverb.setSoundLocation(loc);
//        [self flip];
//        
//        reverb2Pointer->setSoundSourceLoc(loc);
//        
//        self.changeListenerTimer = [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(swapSoundSource:) userInfo:nil repeats:NO];
    }
    
    if (angle > 2 * M_PI){
        angle -= 2*M_PI;
    }
    
    counter ++;
    angle += 0.015;
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
