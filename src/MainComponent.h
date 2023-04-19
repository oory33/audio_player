#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, public juce::ChangeListener, public juce::Timer
{
public:
    //==============================================================================
    MainComponent()
        : state(Stopped)
    {
        addAndMakeVisible(&openButton);
        openButton.setButtonText("Open...");
        openButton.onClick = [this]
        { openButtonClicked(); };

        addAndMakeVisible(&playButton);
        playButton.setButtonText("Play");
        playButton.onClick = [this]
        { playButtonClicked(); };
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled(false);

        addAndMakeVisible(&stopButton);
        stopButton.setButtonText("Stop");
        stopButton.onClick = [this]
        { stopButtonClicked(); };
        stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled(false);

        addAndMakeVisible(&loopingToggle);
        loopingToggle.setButtonText("Loop");
        loopingToggle.onClick = [this]
        { loopButtonChanged(); };

        addAndMakeVisible(&currentPositionLabel);
        currentPositionLabel.setText("Stopped", juce::dontSendNotification);

        setSize(300, 200);

        formatManager.registerBasicFormats();    // [1]
        transportSource.addChangeListener(this); // [2]

        setAudioChannels(0, 2);
        startTimer(20);
    }

    ~MainComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock(bufferToFill);
    }

    void resized() override
    {
        openButton.setBounds(10, 10, getWidth() - 20, 20);
        playButton.setBounds(10, 40, getWidth() - 20, 20);
        stopButton.setBounds(10, 70, getWidth() - 20, 20);
        loopingToggle.setBounds(10, 100, getWidth() - 20, 20);
        currentPositionLabel.setBounds(10, 130, getWidth() - 20, 20);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void changeListenerCallback(juce::ChangeBroadcaster *source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState(Playing);
            else if ((state == Stopping) || (state == Playing))
                changeState(Stopped);
            else if (Pausing == state)
                changeState(Paused);
        }
    }

    void timerCallback() override
    {
        if (transportSource.isPlaying())
        {
            juce::RelativeTime position(transportSource.getCurrentPosition());

            auto minutes = (int)(position.inMinutes()) % 60;
            auto seconds = (int)(position.inSeconds()) % 60;
            auto millis = (int)(position.inMilliseconds()) % 1000;

            auto positionString = juce::String::formatted("%02d:%02d:%03d", minutes, seconds, millis);

            currentPositionLabel.setText(positionString, juce::dontSendNotification);
        }
        else
        {
            currentPositionLabel.setText("Stopped", juce::dontSendNotification);
        }
    }

    void updateLoopState(bool shouldLoop)
    {
        if (readerSource.get() != nullptr)
            readerSource->setLooping(shouldLoop);
    }

    //==============================================================================
    void paint(juce::Graphics &) override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    void changeState(TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
            case Stopped: // [3]
                stopButton.setEnabled(false);
                playButton.setEnabled(true);
                transportSource.setPosition(0.0);
                break;

            case Starting: // [4]
                transportSource.start();
                break;

            case Playing: // [5]
                playButton.setButtonText("Pause");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(true);
                break;

            case Pausing:
                transportSource.stop();
                break;

            case Paused:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Return to Zero");
                break;

            case Stopping: // [6]
                transportSource.stop();
                break;
            }
        }
    }

    void openButtonClicked()
    {
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
                                                      juce::File{},
                                                      "*.wav;*.aif;*.aiff"); // [7]
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        std::cout << chooser << std::endl;
        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser &fc) // [8]
                             {
            auto file = fc.getResult();

            if (file != juce::File{})                                                // [9]
            {
                auto* reader = formatManager.createReaderFor (file);                 // [10]

                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);   // [11]
                    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                    playButton.setEnabled (true);                                                      // [13]
                    readerSource.reset (newSource.release());                                          // [14]
                }
            } });
    }

    void playButtonClicked()
    {
        if ((state == Stopped) || (state == Paused))
        {
            updateLoopState(loopingToggle.getToggleState());
            changeState(Starting);
        }
        else if (state == Playing)
            changeState(Pausing);
    }

    void stopButtonClicked()
    {
        if (state == Paused)
            changeState(Stopped);
        else
            changeState(Stopping);
    }

    void loopButtonChanged()
    {
        updateLoopState(loopingToggle.getToggleState());
    }
    //==============================================================================
    // Your private member variables go here...

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::ToggleButton loopingToggle;
    juce::Label currentPositionLabel;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
