#include "midiJukebox.h"

PlayList::PlayList(char* dir_path, bool aShuffle) {
  // Build the playlist here!
  dirPath = String(dir_path);
  totalNumberOfSongsInDir = countNoOfSongsInDir();

  if(totalNumberOfSongsInDir > songList.size())
  {
    Serial.println("Error: Total number of songs in dir exceeded size of song list!");
    return;
  }

  // This is to loosely "enumerate" these songs one by one...
  fillArrayAscendingNumbers(songListNumbers);

  if(aShuffle)
  {
    isShuffle = true;
    shuffleArray(songListNumbers, totalNumberOfSongsInDir);
    Serial.println("Shuffling playlist!");
  }

  // Fill these list of filenames into the playlist:
  fillPlayListFromDir();
}

uint32_t PlayList::countNoOfSongsInDir() {
  Dir dir = LittleFS.openDir(dirPath.c_str());
  uint32_t noOfSongs = 0;
  // Count number of files:
  while(dir.next()) {
		if (dir.isFile())
      noOfSongs++;
	}
  // Return no. of files inside the directory!
  Serial.printf("Number of files: %d\n",noOfSongs);
  return noOfSongs;
}

void PlayList::fillPlayListFromDir() {
  // https://github.com/littlefs-project/littlefs/issues/972
  Dir dir = LittleFS.openDir(dirPath);
  uint32_t i = 0;

	while(dir.next()) {
		if (dir.isFile()) {
      songList[i] = dir.fileName();
      i++;
		}
	}
}

String PlayList::next() {
  playingSongLocation++;
  if (playingSongLocation >= totalNumberOfSongsInDir)
  {
    return "EndOfDirectory";
  }
  return songList[songListNumbers[playingSongLocation]];
}

String PlayList::getCurrentPlayingSong() {
  return songList[songListNumbers[playingSongLocation]];
}

void PlayList::showFilesInList() {
  for(uint32_t i = 0; i < totalNumberOfSongsInDir; i++)
    Serial.println(songList[i]);
}

// Get random number from the rp2040/rp2350's hardware pseudo-random generator:
uint32_t getRandomNumber(uint32_t min, uint32_t max) {
  uint32_t randomNumber = rp2040.hwrand32();
  return (uint32_t)((randomNumber % (max - min + 1)) + min);
}