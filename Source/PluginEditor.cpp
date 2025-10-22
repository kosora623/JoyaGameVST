/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JoyaGameVSTAudioProcessorEditor::JoyaGameVSTAudioProcessorEditor(JoyaGameVSTAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setWantsKeyboardFocus(true);

    resetGame();

    setSize(600, 400);

    const char* imageData = BinaryData::gameover_png;
    int imageDataSize = BinaryData::gameover_pngSize;
    gameOverImage = juce::ImageFileFormat::loadFrom(imageData, (size_t)imageDataSize);

    bellButton.setButtonText("Test");
    bellButton.addListener(this);

    startTimerHz(60);
}

JoyaGameVSTAudioProcessorEditor::~JoyaGameVSTAudioProcessorEditor()
{
    stopTimer();

    bellButton.removeListener(this);
}

//==============================================================================
void JoyaGameVSTAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);

    auto bounds = getLocalBounds();

    switch (currentState)
    {
        // ------------------
        // 1. スタート画面
        // ------------------
    case GameState::Start:
    {
        auto titleArea = bounds.removeFromTop(bounds.getHeight() * 0.4f);
        g.setFont(50.0f);
        g.drawText("JOYA NO KANE", titleArea, juce::Justification::centred, false);

        auto instructionsArea = bounds;
        g.setFont(18.0f);

        juce::String instructions =
            "Click or press [F] or [J] to Start.\n"
            "Click or press [F] or [J] exactly when the stick hits the rim.\n\n"
            "Miss, and it's Game Over.\n\n\n"
            ;

        g.drawText(instructions, instructionsArea.reduced(20, 0), juce::Justification::centred, true);

        g.setFont(14.0f);
        g.setColour(juce::Colours::grey); // 少し薄い色に
        g.drawText("created by kosora", getLocalBounds().reduced(15), juce::Justification::bottomRight, false);

        break;
    }

    // ------------------
    // 2. カウントダウン画面
    // ------------------
    case GameState::Countdown:
    {
        g.setFont(100.0f);
        g.drawText(juce::String(countdown), bounds, juce::Justification::centred, false);
        break;
    }

    // ------------------
    // 3. ゲーム中画面
    // ------------------
    case GameState::Playing:
    {
        auto bellCenter = getLocalBounds().getCentre().toFloat();
        g.setColour(juce::Colours::darkgoldenrod);
        g.fillEllipse(bellCenter.x - (bellRadius + hitWindow / 2.0f), bellCenter.y - (bellRadius + hitWindow / 2.0f), (bellRadius + hitWindow / 2.0f) * 2.0f, (bellRadius + hitWindow / 2.0f) * 2.0f);
        g.setColour(juce::Colours::grey);
        g.fillEllipse(bellCenter.x - (bellRadius - hitWindow / 2.0f), bellCenter.y - (bellRadius - hitWindow / 2.0f), (bellRadius - hitWindow / 2.0f) * 2.0f, (bellRadius - hitWindow / 2.0f) * 2.0f);

        g.setColour(juce::Colours::sandybrown);
        for (auto* stick : sticks)
        {
            stick->draw(g);
        }

        g.setColour(juce::Colours::white);
        g.setFont(40.0f);
        g.drawText("SCORE: " + juce::String(score), bounds.withTrimmedBottom(50), juce::Justification::centredBottom, false);

        if (specialEffectActive)
        {
            g.setColour(juce::Colours::yellow);
            g.setFont(60.0f);
            g.drawText("BONUS!", getLocalBounds(), juce::Justification::centred, false);
        }

        break;
    }

    // ------------------
    // 4. リザルト画面
    // ------------------
    case GameState::GameOver:
    {
        g.setFont(60.0f);
        g.drawText("GAME OVER", bounds.removeFromTop(100), juce::Justification::centred, false);

        g.setFont(30.0f);
        g.drawText("FINAL SCORE: " + juce::String(score), bounds.removeFromTop(60), juce::Justification::centred, false);

        g.setFont(40.0f);
        juce::String gameOverMessage;

        if (score == 108)
            gameOverMessage = "PERFECT 108!";
        else if (score > 108)
            gameOverMessage = "BONNOU OVERFLOW!";
        else if (score > 80)
            gameOverMessage = "ALMOST CLEANSED!";
        else if (score > 50)
            gameOverMessage = "BONNOU TAKUSAN!";
        else
            gameOverMessage = "MANY BONNOU!";

        g.drawText(gameOverMessage, bounds.removeFromTop(70), juce::Justification::centred, false);

        g.setFont(18.0f);
        g.drawText("Click or press [F] or [J] to return to Title.", bounds.removeFromTop(50), juce::Justification::centred, false);

        if (gameOverImage.isValid())
        {
            int imgW = gameOverImage.getWidth();
            int imgH = gameOverImage.getHeight();

            float scale = 80.0f / (float)imgW;
            int newW = 80;
            int newH = (int)(imgH * scale);

            int xPos = getWidth() - newW - 15;
            int yPos = getHeight() - newH - 15;

            g.drawImage(gameOverImage,
                xPos, yPos, newW, newH,
                0, 0, imgW, imgH); 
        }
        break;
    }
    }
}

void JoyaGameVSTAudioProcessorEditor::resized()
{
}

void JoyaGameVSTAudioProcessorEditor::timerCallback()
{
    if (currentState == GameState::Countdown)
    {
        auto elapsed = juce::Time::getMillisecondCounterHiRes() - countdownStartTime;
        countdown = 3 - (int)(elapsed / 1000.0);

        if (countdown < 1)
        {
            currentState = GameState::Playing;
        }
    }

    if (currentState == GameState::Playing)
    {
        double currentTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        if (currentTime > nextStickSpawnTime)
        {
            spawnStick();

            double interval = juce::jmax(0.5, 2.0 - (score * 0.02));
            nextStickSpawnTime = currentTime + interval;
        }

        auto bellCenter = getLocalBounds().getCentre().toFloat();

        float hitZoneInner = bellRadius - (hitWindow / 2.0f);

        for (int i = sticks.size(); --i >= 0;)
        {
            Stick* stick = sticks.getUnchecked(i);
            stick->update();

            float distance = stick->position.getDistanceFrom(bellCenter);

            float stickTipDistance = distance - 40.0f;

            if (stickTipDistance < hitZoneInner)
            {
                currentState = GameState::GameOver;
                sticks.clear(true);
                break;
            }
        }
    }

    repaint();
}

void JoyaGameVSTAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    handleHitAttempt();
}

bool JoyaGameVSTAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    if (key.getKeyCode() == 'F' || key.getKeyCode() == 'J')
    {
        handleHitAttempt();
        return true;
    }
    return false;
}

void JoyaGameVSTAudioProcessorEditor::handleHitAttempt()
{
    switch (currentState)
    {
    case GameState::Start:
        currentState = GameState::Countdown;
        countdownStartTime = juce::Time::getMillisecondCounterHiRes();
        countdown = 3;
        break;

    case GameState::Playing:
    {
        bool wasHit = false;
        auto bellCenter = getLocalBounds().getCentre().toFloat();

        float hitZoneInner = bellRadius - (hitWindow / 2.0f);
        float hitZoneOuter = bellRadius + (hitWindow / 2.0f);

        for (int i = sticks.size(); --i >= 0;)
        {
            Stick* stick = sticks.getUnchecked(i);
            float distance = stick->position.getDistanceFrom(bellCenter);

            float stickTipDistance = distance - 40.0f;

  
            if (stickTipDistance >= hitZoneInner && stickTipDistance <= hitZoneOuter)
            {
   
                audioProcessor.triggerBellSound();
                score++;

                if (score == 108)
                {
                    specialEffectActive = true;
                }

                sticks.removeObject(stick, true);
                wasHit = true;
                break;
            }
        }

        if (!wasHit)
        {
            currentState = GameState::GameOver;
            sticks.clear(true);
        }
        break;
    }

    case GameState::GameOver:
        resetGame();
        break;

    case GameState::Countdown:
        break;
    }
}

void JoyaGameVSTAudioProcessorEditor::spawnStick()
{
    Stick* newStick = new Stick();

    auto bounds = getLocalBounds();
    auto bellCenter = bounds.getCentre().toFloat();

    float angle = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;

    float distance = (float)bounds.getWidth() / 2 + 100.0f;

    newStick->position.setXY(bellCenter.x + std::cos(angle) * distance,
        bellCenter.y + std::sin(angle) * distance);

    float speed = juce::jmin(4.0f, 2.0f + (score * 0.02f));

    newStick->velocity.setXY(bellCenter.x - newStick->position.x,
        bellCenter.y - newStick->position.y);

    float vectorLength = newStick->velocity.getDistanceFromOrigin();
    if (vectorLength > 0.0f)
    {
        newStick->velocity = newStick->velocity / vectorLength;
    }

    newStick->velocity *= speed;

    newStick->rotation = std::atan2(newStick->velocity.y, newStick->velocity.x) + juce::MathConstants<float>::halfPi;

    sticks.add(newStick);
}

void JoyaGameVSTAudioProcessorEditor::resetGame()
{
    currentState = GameState::Start;
    score = 0;
    countdown = 3;

    sticks.clear(true); 
    nextStickSpawnTime = 0.0;
    specialEffectActive = false;
}

void JoyaGameVSTAudioProcessorEditor::buttonClicked(juce::Button* button)
{
}