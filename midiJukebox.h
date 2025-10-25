#ifndef MIDIJUKEBOX_H
#define MIDIJUKEBOX_H

#include <vector>
#include <LittleFS.h>
#include <MD_MIDIFile.h>

class PlayList {
  public:
  PlayList(char* dir_path, bool aShuffle);
  uint32_t countNoOfSongsInDir();
  void fillPlayListFromDir();
  void showFilesInList();
  inline uint32_t getPlayingSongLocation() { return playingSongLocation; };

  String getCurrentPlayingSong();
  String next();
  
  private:
  bool isShuffle = true;
  const uint32_t MAX_NUMBER_SONGS = 64;
  String dirPath = "/";
  std::vector<uint32_t> songListNumbers = std::vector<uint32_t>(MAX_NUMBER_SONGS, 0);
  std::vector<String> songList = std::vector<String>(MAX_NUMBER_SONGS);
  uint32_t totalNumberOfSongsInDir = 0;
  uint32_t playingSongLocation = 0;
};

template<typename T>
void shuffleArray(std::vector<T>& aArray) {
  // https://documents.uow.edu.au/~lukes/textbook/notes-cpp/misc/random-shuffle.html
  for (uint32_t i = 0; i < aArray.size(); i++) {
    uint32_t r = rp2040.hwrand32() % aArray.size();  // generate a random position
    T temp = aArray[i];
    aArray[i] = aArray[r];
    aArray[r] = temp;
  }
}

template<typename T>
void shuffleArray(std::vector<T>& aArray, size_t aSize) {
  // https://documents.uow.edu.au/~lukes/textbook/notes-cpp/misc/random-shuffle.html
  for (uint32_t i = 0; i < aSize; i++) {
    uint32_t r = rp2040.hwrand32() % aSize;  // generate a random position
    T temp = aArray[i];
    aArray[i] = aArray[r];
    aArray[r] = temp;
  }
}

template<typename T>
void fillArrayAscendingNumbers(std::vector<T>& aArray) {
  for (uint32_t i = 0; i < aArray.size(); i++) {
     aArray[i] = i;
  }
}

uint32_t getRandomNumber(uint32_t min, uint32_t max);
#endif