#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <iostream>
#include <array>
#include <crypto++/cryptlib.h>
#include <crypto++/md5.h>
#include <crypto++/sha.h>
#include <random>
#include <string>
#include <crypto++/md4.h>
#include <crypto++/hex.h>
#include <crypto++/base64.h>
#include <regex>
#include <csignal>
#include <thread>

//const std::array<char, 80> {'a', 'b', 'c''d''e''f''g''h''i''j''k''l''m''n''o''p''q''r''s''t''u''v''w''x''y''z''A''B''C''D''E''F''G''H''I''J''K''L''M''N''O''P''Q''R''S''T''U''V''W''X''Y''Z'};
constexpr int charArrSize = 63;
const std::array<char, charArrSize> alph {"abcdefghijklmno1pqrs2tuv3wx4yz5ABCD6EFGH7IJKL8MNOPQ9RSTU0VWXYZ"};

auto threads_and_stops = std::vector<std::pair<std::thread, std::shared_ptr<bool>>>{};

void signalHandler( int signum ) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Joining threads\n";
    int i = 0;
    for (auto &p : threads_and_stops) {
        (*p.second) = false;
        p.first.join();
        std::cout << "thread " << ++i << " joined\n";
    }
    // cleanup and close up stuff here  
    // terminate program  
    std::cout << "closing\n";
    exit(signum);  
}

void temp(std::mt19937 &gen, std::uniform_int_distribution<> &distrib, const std::regex &rx, bool *running) {
    using namespace CryptoPP;

    std::string message;
    message.resize(16);
    byte digest[SHA256::DIGESTSIZE];
    std::string output;
    Base64Encoder b64;
    b64.Attach(new CryptoPP::StringSink(output));

    while (*running) {
        for (int n = 0; n < 16; ++n)
            //Use `distrib` to transform the random unsigned int generated by gen into an int in [0, 52]
            message[n] = alph[distrib(gen)];

        SHA256().CalculateDigest(digest, reinterpret_cast<const byte *>(message.data()), message.size());

        b64.Put(digest, sizeof(digest));
        b64.MessageEnd();

        auto matches = std::distance(
            std::sregex_iterator(output.begin(), output.end(), rx),
            std::sregex_iterator());
        if (matches > 0) {
            std::cout << "trip: #" << output.substr(0,6) << " password: #" << message << std::endl;
        }
        output.clear();
    }
}

void init(const std::regex &rx, bool *running) {
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0, charArrSize-2);

    temp(gen, distrib, rx, running);
}

int main() {
    std::string message;
    int32_t number_of_threads = 0;
    signal(SIGINT, signalHandler);

    std::cout << "input regex to search for: ";
    std::cin >> message;

    std::cout << "input number of cores/threads to use: ";
    std::cin >> number_of_threads;

    if (number_of_threads > std::thread::hardware_concurrency()) {
        number_of_threads = std::thread::hardware_concurrency();
    }

    threads_and_stops.reserve(number_of_threads);
    std::cout << "Pattern: " << message;
    std::cout << "\nThreads: " << number_of_threads << std::endl;

    std::regex rx(message);

    std::cout << "init ...";
    for (int i = 0; i < number_of_threads; ++i) {
        std::shared_ptr<bool> running = std::make_shared<bool>(true);
        bool *r = running.get();
        std::pair<std::thread, std::shared_ptr<bool>> pair = std::make_pair(std::move(std::thread(init, rx, r)), std::move(running));
        threads_and_stops.push_back(std::move(pair));
    }

    std::cout << "\nrunning" << std::endl;
    for (auto &p : threads_and_stops) {
        //avoids ending the main thread. Program will close when it receives the interrupt signal
        p.first.join();
    }

    return 0;
}