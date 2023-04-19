#include "MainComponent.h"

//==============================================================================

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

// void MainComponent::resized() override
// {
//     // This is called when the MainComponent is resized.
//     // If you add any child components, this is where you should
//     // update their positions.
// }
