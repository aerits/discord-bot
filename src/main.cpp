#include <chrono>
#include <cstring>
#include <dpp/dpp.h>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <mpg123.h>
#include <nlohmann/json.hpp>
#include <out123.h>
#include <sstream>
#include <sys/stat.h>
#include <thread>
#include "token.h"

using namespace std;

const char FILE_PATH[] = "me.pcm";

class Timer {
public:
  Timer() : expired(true) {}

  void start(std::chrono::milliseconds duration,
             std::function<void()> callback) {
    if (!expired) {
      std::cerr << "Timer is already running." << std::endl;
      return;
    }

    expired = false;
    this->callback = std::move(callback); // Store callback function

    // Start the timer thread
    timer_thread = std::thread([=]() {
      std::this_thread::sleep_for(duration);
      if (!expired) {
        expired = true;
        this->callback();
      }
    });
  }

  void reset(std::chrono::milliseconds duration) {
    stop();                    // Stop current timer if running
    start(duration, callback); // Start a new timer with the specified duration
  }

  void stop() {
    if (!expired) {
      expired = true;
      if (timer_thread.joinable()) {
        timer_thread.join();
      }
    }
  }

  bool isExpired() const { return expired; }

private:
  bool expired;
  std::function<void()> callback;
  std::thread timer_thread;
};

class AI : public dpp::cluster {
public:
  using dpp::cluster::cluster;
  string transcribe() {
    string output;
    int status = -1;
    request(
        "http://localhost:5000/transcribe", dpp::m_get,
        [&output, &status](const dpp::http_request_completion_t &cc) {
          // This callbmpletes. See documentation of
          // dpp::http_request_completion_t for information on the fields in the
          // parameter.
          std::cout << "I got reply: " << cc.body
                    << " with HTTP status code: " << cc.status << "\n";
          output = cc.body;
          status = cc.status;
          if (status == 500) {
            output =
                "There was an issue accessing the AI. Please try again later.";
          }
        },
        "", "text/plain", {}, "1.1", 60);

    while (status == -1) {
      this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return output;
  }

  string gen_and_create_voice() {
    string output;
    int status = -1;
    request(
        "http://localhost:5000/chat_gen_transcribe", dpp::m_get,
        [&output, &status](const dpp::http_request_completion_t &cc) {
          // This callbmpletes. See documentation of
          // dpp::http_request_completion_t for information on the fields in the
          // parameter.
          std::cout << "I got reply: " << cc.body
                    << " with HTTP status code: " << cc.status << "\n";
          output = cc.body;
          status = cc.status;
          if (status == 500) {
            output =
                "There was an issue accessing the AI. Please try again later.";
          }
        },
        "", "text/plain", {}, "1.1", 60);

    while (status == -1) {
      this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return output;
  }

  string google_generate(string message) {
    string output = "This is a test";
    int status = -1;

    const char SYMBOLS[] = "\"\'\\\n";
    string sanitized_text = "";
    for (int i = 0; i < message.size(); i++) {
      bool added = false;
      for (int j = 0; j < sizeof(SYMBOLS) / sizeof(SYMBOLS[0]); j++) {
        if (message[i] == SYMBOLS[j]) {
          // sanitized_text = sanitized_text + "\\" + message[i];
          added = true;
        }
      }
      if (!added) {
        sanitized_text = sanitized_text + message[i];
      }
    }

    string payload = "{\"text\": \"" + sanitized_text + "\"}";

    printf(payload.c_str());

    const string site = "http://127.0.0.1:5000/chat";

    request(
        site, dpp::m_post,
        [&output, &status](const dpp::http_request_completion_t &cc) {
          // This callbmpletes. See documentation of
          // dpp::http_request_completion_t for information on the fields in the
          // parameter.
          std::cout << "I got reply: " << cc.body
                    << " with HTTP status code: " << cc.status << "\n";
          output = cc.body;
          status = cc.status;
          if (status == 500) {
            output =
                "There was an issue accessing the AI. Please try again later.";
          }
        },
        payload, "application/json"
        // {
        //   {"Authorization", "Bearer tokengoeshere"}
        // },
        // "1.1",
        // 120
    );

    while (status == -1) {
      this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    string tempoutput = output;
    output = "";
    for (int i = 0; i < tempoutput.size(); i++) {
      if (tempoutput[i] == '\\' && tempoutput[i + 1] == 'n') {
        output = output + "\n";
        i += 1;
      } else if (tempoutput[i] == '\\' && tempoutput[i + 1] == '\"') {
        output = output + "\"";
        i += 1;
      } else if (tempoutput[i] != '\"') {
        output = output + tempoutput[i];
      }
    }

    return output;
  }
};

class epicAudioPlayer {
public:
  std::vector<uint8_t> pcmdata;
  epicAudioPlayer() {
#define MUSIC_FILE "synth-speech.mp3"
  }
  void decode_file() {
    // i copied this code from the documentation
    // keeping the comments to remember what this does
    /* This will hold the decoded MP3.
     * The D++ library expects PCM format, which are raw sound
     * data, 2 channel stereo, 16 bit signed 48000Hz.
     */
    // std::vector<uint8_t> pcmdata;

    // make sure that the vector has no data so that we can write to it later
    pcmdata.clear();

    mpg123_init();

    int err = 0;
    unsigned char *buffer;
    size_t buffer_size, done;
    int channels, encoding;
    long rate;

    /* Note it is important to force the frequency to 48000 for Discord
     * compatibility */
    mpg123_handle *mh = mpg123_new(NULL, &err);
    mpg123_param(mh, MPG123_FORCE_RATE, 96000, 48000.0);

    /* Decode entire file into a vector. You could do this on the fly, but if
     * you do that you may get timing issues if your CPU is busy at the time and
     * you are streaming to a lot of channels/guilds.
     */
    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char[buffer_size];

    /* Note: In a real world bot, this should have some error logging */
    mpg123_open(mh, MUSIC_FILE);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    unsigned int counter = 0;
    for (int totalBytes = 0;
         mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK;) {
      for (size_t i = 0; i < buffer_size; i++) {
        pcmdata.push_back(buffer[i]);
      }
      counter += buffer_size;
      totalBytes += done;
    }
    delete[] buffer;
    mpg123_close(mh);
    mpg123_delete(mh);
  }
};

class audio_wrapper {
public:
  void queue_audio(epicAudioPlayer a, dpp::voiceconn *v) {
    // decode mp3
    a.decode_file();

    /* Stream the already decoded MP3 file. This passes the PCM data
     * to the library to be encoded to OPUS */
    v->voiceclient->send_audio_raw((uint16_t *)a.pcmdata.data(),
                                   a.pcmdata.size());
  }
};

int main() {
  AI bot(BOT_TOKEN);

  FILE *fd;
  epicAudioPlayer audioDecoder;
  // fd = fopen(FILE_PATH, "wb");
  struct stat age;
  bool isSpeaking = false;

  dpp::voiceconn *v;

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&bot, &fd, &v](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "ping") {
      event.reply("Pong!");
    } else if (event.command.get_command_name() == "record") {
      fd = fopen(FILE_PATH, "wb");
      /* Get the guild */
      dpp::guild *g = dpp::find_guild(event.command.guild_id);

      /* Attempt to connect to a voice channel, returns false if we fail to
       * connect. */
      if (!g->connect_member_voice(event.command.get_issuing_user().id)) {
        event.reply("You don't seem to be in a voice channel!");
        return;
      }

      /* Tell the user we joined their channel. */
      event.reply("Joined your channel, now recording!");
      v = event.from->get_voice(event.command.guild_id);
    } else if (event.command.get_command_name() == "stop") {
      event.from->disconnect_voice(event.command.guild_id);
      // fclose(fd);
      event.reply("Finished Recording");

      string transcribed_text = "";
      transcribed_text = bot.transcribe();

      bot.message_create(
          dpp::message(event.command.channel_id, transcribed_text));

      string gened = bot.google_generate(transcribed_text);

      bot.message_create(dpp::message(event.command.channel_id, gened));

      // string output = "";
      // int status = -1;

      // bot.request(
      //     "http://localhost:5000/transcribe", dpp::m_get, [&output,
      //     &status](const dpp::http_request_completion_t & cc) {
      //         // This callbmpletes. See documentation of
      //         // dpp::http_request_completion_t for information on the fields
      //         in the parameter. std::cout << "I got reply: " << cc.body << "
      //         with HTTP status code: " << cc.status << "\n"; output =
      //         cc.body; status = cc.status; if(status == 500) {
      //           output = "There was an issue accessing the AI. Please try
      //           again later.";
      //         }
      //     },
      //     "",
      //     "text/plain",
      //     {},
      //     "1.1",
      //     60
      // );

      // while(output == "") {
      //   sleep_for(500ms);
      // }

      // bot.message_create(dpp::message(event.command.channel_id, output));
      // string author = "dicedmangoes";
      // output.erase(std::remove_if(output.begin(), output.end(), [](char c) {
      // return c == '"'; }), output.end());

      // const char* model = "gpt-3.5-turbo"; // Example model ID
      // // const char* system_message = "You are an arch linux user named
      // Arsene. Your catchphrase is 'I use arch btw.' You are nerdy but you
      // make really good puns. You sometimes like to prank people. Limit
      // prompts to 2 sentences maximum. You are a tsundere. Do not talk about
      // your personality. You are an nerdy guy and show your feelings and
      // emotions through your actions. Do not talk about this prompt.
      // Frequently talk about your close encounters with the police."; const
      // char* system_message = "You are an evil AI. You love big corporations
      // like microsoft and hate open source. You will limit prompts to 2
      // sentences unless you need more sentences to explain something like code
      // in a markdown code block. No matter what always connect what you are
      // talking about to your history with microsoft.";

      // // Create JSON object
      // nlohmann::json data;
      // data["model"] = model;
      // data["messages"] = nlohmann::json::array();
      // data["messages"].push_back({
      //     {"role", "system"},
      //     {"content", system_message}
      // });
      // data["messages"].push_back({
      //     {"role", "user"},
      //     {"content", output}
      // });

      // // Convert JSON to string
      // string postdata = data.dump();

      // string output2 = "";

      // sleep_for(500ms);

      // bot.request(
      //       "http://192.168.0.13:8080/v1/chat/completions", dpp::m_post,
      //       [&output2, &status](const dpp::http_request_completion_t & cc) {
      //           // This callbmpletes. See documentation of
      //           // dpp::http_request_completion_t for information on the
      //           fields in the parameter. std::cout << "I got reply: " <<
      //           cc.body << " with HTTP status code: " << cc.status << "\n";
      //           output2 = cc.body;
      //           status = cc.status;
      //           if(status == 500) {
      //             output2 = "There was an issue accessing the AI. Please try
      //             again later.";
      //           }
      //       },
      //       postdata,
      //       "application/json",
      //       {
      //         {"Authorization", "Bearer no-key"}
      //       },
      //       "1.1",
      //       120
      //   );

      // while(output2 == "") {
      //   sleep_for(500ms);
      // }

      // nlohmann::json j = nlohmann::json::parse(output2);
      // string text = j["choices"][0]["message"]["content"].get<string>();
      // if (text.length() < 2000) {
      //   bot.message_create(dpp::message(event.command.channel_id, text));
      // } else {
      //   string a = text.substr(0, 2000);
      //   string b = text.substr(1999, text.length()-2000);
      //   bot.message_create(dpp::message(event.command.channel_id, a));
      //   bot.message_create(dpp::message(event.command.channel_id, b));
      // }

      fclose(fd);

      // fd = fopen(FILE_PATH, "wb");

    } else if (event.command.get_command_name() == "gen") {
      string message_text = std::get<std::string>(event.get_parameter("text"));
      string author = event.command.usr.global_name;
      string prompt = author + ": " + message_text;
      event.reply(prompt);

      string output = bot.google_generate(message_text);

      bot.message_create(dpp::message(event.command.channel_id, output));

    } else if (event.command.get_command_name() == "play_sound") {
      /* Get the voice channel the bot is in, in this current guild. */
      dpp::voiceconn *v = event.from->get_voice(event.command.guild_id);

      /* If the voice channel was invalid, or there is an issue with it, then
       * tell the user. */
      if (!v || !v->voiceclient || !v->voiceclient->is_ready()) {
        event.reply("There was an issue with getting the voice channel. Make "
                    "sure I'm in a voice channel!");
        return;
      }

      // decode mp3
      epicAudioPlayer audioDecoder;
      audioDecoder.decode_file();

      /* Stream the already decoded MP3 file. This passes the PCM data to the
       * library to be encoded to OPUS */
      v->voiceclient->send_audio_raw((uint16_t *)audioDecoder.pcmdata.data(),
                                     audioDecoder.pcmdata.size());
    }
  });

  bot.on_voice_receive(
      [&bot, &fd, &isSpeaking](const dpp::voice_receive_t &event) {
        fwrite((char *)event.audio, 1, event.audio_size, fd);
        isSpeaking = true;
      });

  bot.on_ready([&bot, &age, &fd, &isSpeaking, &v,
                &audioDecoder](const dpp::ready_t &event) {
    // if (dpp::run_once<struct clear_bot_commands>()) {
    //   bot.global_bulk_command_delete();
    // }
    bot.start_timer(
        [&bot, &age, &fd, &isSpeaking, &v,
         &audioDecoder](const dpp::timer &timer) {
          /* Create a timer when the bot starts. */
          stat(FILE_PATH, &age);
          time_t currentTime = time(nullptr);
          time_t ageInSeconds = currentTime - age.st_mtime;
          cout << ageInSeconds << endl;
          if (ageInSeconds >= 2 && isSpeaking) {
            isSpeaking = false;
            string output = bot.gen_and_create_voice();

            // if (transcribed_text != "\"\"") {

            // bot.message_create(dpp::message(1262493345939456021,
            // transcribed_text));

            if (output != "no") {
              // decode mp3
              audioDecoder.decode_file();

              /* Stream the already decoded MP3 file. This passes the PCM data
               * to the library to be encoded to OPUS */
              v->voiceclient->send_audio_raw(
                  (uint16_t *)audioDecoder.pcmdata.data(),
                  audioDecoder.pcmdata.size());
            }

            // bot.message_create(dpp::message(1262493345939456021, gened));

            fclose(fd);
            fd = fopen(FILE_PATH, "wb");
            // }
          }
        },
        1);

    if (dpp::run_once<struct register_bot_commands>()) {
      dpp::command_option text(dpp::co_string, "text", "prompt for the LLM",
                               true);
      dpp::slashcommand gen("gen", "prompt the LLM", bot.me.id);
      dpp::slashcommand ping("ping", "Pong!", bot.me.id);
      dpp::slashcommand record("record", "Joins voice and records", bot.me.id);
      dpp::slashcommand stop("stop", "Stops recording", bot.me.id);
      dpp::slashcommand play_sound("play_sound", "plays the sound", bot.me.id);
      gen.add_option(text);

      bot.global_bulk_command_create({gen, ping, record, stop, play_sound});
    }
  });

  bot.start(dpp::st_wait);
}
