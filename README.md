# Voice Activated AI Chatbot In Discord

This is a project that I created because I saw this vtuber called "Neuro-sama" which is an AI vtuber. I noticed in clips that Vedal, Neuro-sama's creator, mentioned to collab partners to 
join his Neuro-sama discord and things like seeing Neuro-sama in a discord voice chat with Vedal. When I realized that this meant that Neuro-sama is a discord bot, I started thinking
about how I could make my own version of that.

This project is a discord bot that you can talk to in a discord voice channel.

# Features
- you can talk to it with either the `/record` or `/gen <text>`
- it runs fast

# Features that are being worked on
- way to interrupt the AI using
- more realistic AI voice
- better latency

# Architecture

This has a python flask server that runs an api that the discord bot in C++ can contact. The bot records what you say and has a timer waiting for you to not speak for 2 seconds. 
After you don't speak for 2 seconds, it contacts the python server to transcribe the text. This audio is saved in PCM format with 48000 sample rate 2 channel audio because
that is the format that discord sends.

The python server first uses a locally running [whisper](https://github.com/openai/whisper) to
transcribe what you said and then
contacts the [Google AI studio](https://ai.google.dev/aistudio) api to generate 
a response to what you said. 

It then creates tts of the AI response and writes it to an mp3 and then it finishes the api call. The discord bot which was waiting for the response now converts the mp3 file
into PCM audio and then encodes it as opus and sends it to discord to play in the voice channel.

# Dependencies
C++
- latest version of D++ (you should compile it)
- libmpg123

Python
- edge_tts
- flask
- google_generativeai
- pydub
- wave
- whisper

# Compiling and running

```bash
cmake --build build
```

## Necessary files to add

Ok I'm too lazy to use env variables. There are two files you need to create to add api keys. One is for the discord bot and the other is for the Google AI studio api key

Python
```py
api_key.py

api_key = "insert api key"
```

C++
```C++
src/token.cpp

#include "token.h"

const char BOT_TOKEN[75] = "insert discord bot token";

```

## Running

```bash
flask run
```

Then open another terminal

```bash
./build/discord-bot
```

## Using in discord

```
/record
```

To join a voice channel.

At the moment it can't leave yet.
