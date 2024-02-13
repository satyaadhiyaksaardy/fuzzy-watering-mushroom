#ifndef TELEGRAMBOTMANAGER_H
#define TELEGRAMBOTMANAGER_H

#include "FuzzyLogicController.h"
#include "ActuatorControl.h"
#include "Globals.h"

class TelegramBotManager
{
public:
    static void init()
    {
        bot.begin();
    }

    static void handleMessages()
    {
        nowMillis = millis();
        if (nowMillis > botLastScan + (botScanInterval * 1000))
        {
            botLastScan = millis();

            bot.getUpdates(bot.message[0][1]); // launch API GetUpdates up to xxx message
            // loop at messages received
            for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)
            {
                handleBotCommands(i);
            }
            bot.message[0][0] = ""; // All messages have been replied - reset new messages
        }
    }

    static void sendErrorNotification(int id)
    {
        botResponse = "Ada masalah di Sensor Node Nomor " + String(id) + ", Segera cek ke kumbung!";
        for (size_t i = 0; i < userCount; i++)
        {
            bot.sendMessage(users[i], botResponse, "");
        }
    }

    static void sendWateringNotification(String messages)
    {
        for (size_t i = 0; i < userCount; i++)
        {
            bot.sendMessage(users[i], messages, "");
        }
    }

private:
    static void handleBotCommands(int line)
    {
        botCommand = bot.message[line][5]; // message reiceived
        botCommand.toUpperCase();          // not case sensitive anymore
        chatId = bot.message[line][4];
        sender = bot.message[line][2];

        if (botCommand.equals("/START"))
        {
            botResponse = "Aku adalah bot yang akan membantumu memonitoring dan merawat kumbung jamur kamu, Senang kenal kamu " + sender + ".";
            botResponse.concat("%0A");
            botResponse.concat("Silahkan ketik /help untuk melihat daftar perintah yang aku pahami.");
        }
        else if (botCommand.equals("/HELP"))
        {
            botResponse = "Perintah yang aku pahami : ";
            botResponse.concat("%0A");
            botResponse.concat("%0A");
            botResponse.concat("/start - Memulai bot");
            botResponse.concat("%0A");
            botResponse.concat("/mode - Mengubah mode penyiraman manual/otomatis");
            botResponse.concat("%0A");
            botResponse.concat("/condition - Membaca suhu dan kelembaban ruangan");
            botResponse.concat("%0A");
            botResponse.concat("/water - Menyirami jamur anda");
            botResponse.concat("%0A");
            botResponse.concat("/help - Melihat panduan ini");
            botResponse.concat("%0A");
            botResponse.concat("/ip - Melihat alamat IP lokal");
        }
        else if (botCommand.equals("/CONDITION"))
        {
            botResponse = "Suhu : ";
            botResponse.concat(globalData.averageTemperature);
            botResponse.concat("°C");
            botResponse.concat("%0A");
            botResponse.concat("Kelembaban : ");
            botResponse.concat(globalData.averageHumidity);
            botResponse.concat("%");
        }
        else if (botCommand.equals("/WATER"))
        {
            if (!globalData.autoWateringEnabled)
            {
                if (!globalData.isWatering)
                {
                    FuzzyLogicController::processFuzzy();
                    globalData.isWatering = true;
                    globalData.wateringStartTime = millis(); // Mark start time
                    ActuatorControl::controlPump(true);      // Start watering
                    botResponse = "Mulai menyiram selama " + String(globalData.wateringDuration / 1000) + " detik";
                }
                else
                {
                    botResponse = "Penyiraman sedang berjalan.";
                }
            }
            else
            {
                botResponse = "Penyiraman diatur ke mode otomatis, silahkan pindah ke mode manual terlebih dahulu.";
            }
        }
        else if (botCommand.equals("/MODE"))
        {
            globalData.autoWateringEnabled = !globalData.autoWateringEnabled;

            String mode = globalData.autoWateringEnabled ? "otomatis" : "manual";
            botResponse = "Mode penyiraman diubah ke *" + mode + "*!";
        }
        else if (botCommand.equals("/IP"))
        {
            botResponse = "Alamat IP : ";
            botResponse += WiFi.localIP().toString();
        }
        else
        {
            botResponse = "Maaf " + sender + ", Aku belum paham perintah itu, silahkan ketik /help untuk melihat daftar perintah yang tersedia";
        }

        bot.sendMessage(chatId, botResponse, "");
    }
};

#endif
