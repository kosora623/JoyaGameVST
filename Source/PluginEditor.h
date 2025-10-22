/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct Stick
{
    juce::Point<float> position;
    juce::Point<float> velocity;
    float rotation;

    void update()
    {
        position += velocity;
    }

    void draw(juce::Graphics& g)
    {
        juce::Path stickPath;
        stickPath.addRectangle(-5.0f, -40.0f, 10.0f, 80.0f);

        g.fillPath(stickPath, juce::AffineTransform::rotation(rotation)
            .translated(position));
    }
};

//==============================================================================

class JoyaGameVSTAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer,
    public juce::Button::Listener
{
public:
    enum class GameState
    {
        Start, 
        Countdown, 
        Playing, 
        GameOver
    };

    JoyaGameVSTAudioProcessorEditor(JoyaGameVSTAudioProcessor&);
    ~JoyaGameVSTAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& event) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void buttonClicked(juce::Button* button) override;

private:
    JoyaGameVSTAudioProcessor& audioProcessor;

    GameState currentState; 
    int score;
    int countdown;
    double countdownStartTime; // changed from juce::int64 to double to avoid precision warnings
    const float bellRadius = 80.0f;
    const float hitWindow = 25.0f;
    bool specialEffectActive;

    juce::Image gameOverImage;
    juce::TextButton bellButton;
    juce::OwnedArray<Stick> sticks;

    double nextStickSpawnTime;
    bool fKeyDown = false;
    bool jKeyDown = false;

    void handleHitAttempt(); 
    void resetGame();
    void spawnStick();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JoyaGameVSTAudioProcessorEditor)
};