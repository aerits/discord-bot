from pydub import AudioSegment
import wave
import whisper

model = whisper.load_model("base")

def pcm_to_wav(pcm_file_path, wav_file_path, channels=2, sample_width=2, frame_rate=48000):
    # Read PCM data
    with open(pcm_file_path, 'rb') as pcm_file:
        pcm_data = pcm_file.read()

    # Create a temporary WAV file with the PCM data
    with wave.open(wav_file_path, 'wb') as wav_file:
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(sample_width)
        wav_file.setframerate(frame_rate)
        wav_file.writeframes(pcm_data)

    # Load the temporary WAV file and resample if needed
    audio = AudioSegment.from_wav(wav_file_path)
    if audio.frame_rate != frame_rate:
        audio = audio.set_frame_rate(frame_rate)
    audio.export(wav_file_path, format='wav')

# Transcribe the audio
def transcribe_audio(wav_file_path):
    result = model.transcribe(wav_file_path)
    print("Transcription: ", result['text'])
    return result['text']

def main():
    # Convert PCM to WAV with correct parameters
    pcm_to_wav('me.pcm', 'output.wav')

    # Transcribe the audio
    print(transcribe_audio('output.wav'))

