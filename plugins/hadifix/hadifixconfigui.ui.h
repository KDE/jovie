/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int percentToSlider (int percentValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int sliderToPercent (int sliderValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void volumeBox_valueChanged (int percentValue) {
   volumeSlider->setValue (percentToSlider (percentValue));
}

void timeBox_valueChanged (int percentValue) {
   timeSlider->setValue (percentToSlider (percentValue));
}

void frequencyBox_valueChanged (int percentValue) {
   frequencySlider->setValue (percentToSlider (percentValue));
}

void volumeSlider_valueChanged (int sliderValue) {
   volumeBox->setValue (sliderToPercent (sliderValue));
}

void timeSlider_valueChanged (int sliderValue) {
   timeBox->setValue (sliderToPercent (sliderValue));
}

void frequencySlider_valueChanged (int sliderValue) {
   frequencyBox->setValue (sliderToPercent (sliderValue));
}

void init () {
   male = KIconLoader::global()->loadIcon("male", K3Icon::Small);
   female = KIconLoader::global()->loadIcon("female", K3Icon::Small);
}

void addVoice (const QString &filename, bool isMale) {
   if (isMale) {
      if (!maleVoices.contains(filename)) {
         int id = voiceCombo->count();
         maleVoices.insert (filename, id);
         voiceCombo->addItem (male, filename);
      }
   }
   else {
      if (!femaleVoices.contains(filename)) {
         int id = voiceCombo->count();
         femaleVoices.insert (filename, id);
         voiceCombo->addItem (female, filename);
      }
   }
}

void addVoice (const QString &filename, bool isMale, const QString &displayname) {
   addVoice (filename, isMale);
   
   if (isMale) {
      defaultVoicesMap [maleVoices [filename]] = filename;
      voiceCombo->setItemIcon (maleVoices [filename], male);
      voiceCombo->setItemText (maleVoices [filename], displayname);
   }
   else{
      defaultVoicesMap [femaleVoices [filename]] = filename;
      voiceCombo->setItemIcon (femaleVoices [filename], female);
      voiceCombo->setItemText (femaleVoices [filename], displayname);
   }
}

void setVoice (const QString &filename, bool isMale) {
   addVoice (filename, isMale);
   if (isMale)
      voiceCombo->setCurrentIndex (maleVoices[filename]);
   else
      voiceCombo->setCurrentIndex (femaleVoices[filename]);
}

QString getVoiceFilename() {
   int curr = voiceCombo->currentIndex();

   QString filename = voiceCombo->itemText(curr);
   if (defaultVoicesMap.contains(curr))
      filename = defaultVoicesMap[curr];

   return filename;
}

bool isMaleVoice() {
   int curr = voiceCombo->currentIndex();
   QString filename = getVoiceFilename();

   if (maleVoices.contains(filename))
      return maleVoices[filename] == curr;
   else
      return false;
}

void changed (const QString &) {
   emit changed (true);
}
