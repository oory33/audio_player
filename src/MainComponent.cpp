#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : state(Stopped)
{
    setSize(600, 400);
    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.onClick = [this]
    {
        openButtonCliked();
    };

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this]
    {
        playButtonClicked();
    };
    playButton.setColor(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]
    {
        stopButtonClicked();
    };
    stopButton.setColor(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);
}

//==============================================================================
void MainComponent::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::Font(16.0f));
    g.setColour(juce::Colours::white);
    g.drawText("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
