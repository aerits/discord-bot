import subprocess

import edge_tts
import google.generativeai as genai
from flask import Flask, jsonify, request

import stt

from api_key import api_key

genai.configure(api_key=api_key)
model = genai.GenerativeModel(
    model_name="gemini-1.5-flash",
    system_instruction="""
    You are in a discord call.
    You are an jokester but you are very helpful.
    Do not use emojis.
    Limit messages to 1 sentence.
    """,
)
_chat = model.start_chat(history=[])


app = Flask(__name__)


@app.route("/chat", methods=["POST"])
def chat():
    data = request.get_json()
    print(data)
    output = _chat.send_message(data["text"])
    return jsonify(output.text)


@app.route("/chat_gen_transcribe", methods=["GET"])
def chatGenTranscribe():
    stt.pcm_to_wav("me.pcm", "output.wav")
    transcribed: str = stt.transcribe_audio("output.wav")
    try:
        if len(transcribed) > 2:
            output: str = _chat.send_message(transcribed).text
            communicate = edge_tts.Communicate(output, "en-GB-SoniaNeural")
            communicate.save_sync("synth-speech.mp3")
            return jsonify("completed")
        else:
            raise ValueError("Nothing to transcribe.")
    except:
        communicate = edge_tts.Communicate(
            "There was an error with the AI", "en-GB-SoniaNeural"
        )
        communicate.save_sync("synth-speech.mp3")
    return jsonify("no")


@app.route("/transcribe", methods=["GET"])
def transcribe():
    stt.pcm_to_wav("me.pcm", "output.wav")
    return jsonify(stt.transcribe_audio("output.wav"))


if __name__ == "__main__":
    app.run(debug=True)
