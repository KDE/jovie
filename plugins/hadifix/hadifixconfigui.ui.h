/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use TQt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int HadifixConfigUI::percentToSlider (int percentValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int HadifixConfigUI::sliderToPercent (int sliderValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void HadifixConfigUI::volumeBox_valueChanged (int percentValue) {
   volumeSlider->setValue (percentToSlider (percentValue));
}

void HadifixConfigUI::timeBox_valueChanged (int percentValue) {
   timeSlider->setValue (percentToSlider (percentValue));
}

void HadifixConfigUI::frequencyBox_valueChanged (int percentValue) {
   frequencySlider->setValue (percentToSlider (percentValue));
}

void HadifixConfigUI::volumeSlider_valueChanged (int sliderValue) {
   volumeBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfigUI::timeSlider_valueChanged (int sliderValue) {
   timeBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfigUI::frequencySlider_valueChanged (int sliderValue) {
   frequencyBox->setValue (sliderToPercent (sliderValue));
}

void HadifixConfigUI::init () {
   male = KGlobal::iconLoader()->loadIcon("male", KIcon::Small);
   female = KGlobal::iconLoader()->loadIcon("female", KIcon::Small);
}

void HadifixConfigUI::addVoice (const TQString &filename, bool isMale) {
   if (isMale) {
      if (!maleVoices.contains(filename)) {
         int id = voiceCombo->count();
         maleVoices.insert (filename, id);
         voiceCombo->insertItem (male, filename, id);
      }
   }
   else {
      if (!femaleVoices.contains(filename)) {
         int id = voiceCombo->count();
         femaleVoices.insert (filename, id);
         voiceCombo->insertItem (female, filename, id);
      }
   }
}

void HadifixConfigUI::addVoice (const TQString &filename, bool isMale, const TQString &displayname) {
   addVoice (filename, isMale);
   
   if (isMale) {
      defaultVoices [maleVoices [filename]] = filename;
      voiceCombo->changeItem (male, displayname, maleVoices [filename]);
   }
   else{
      defaultVoices [femaleVoices [filename]] = filename;
      voiceCombo->changeItem (female, displayname, femaleVoices [filename]);
   }
}

void HadifixConfigUI::setVoice (const TQString &filename, bool isMale) {
   addVoice (filename, isMale);
   if (isMale)
      voiceCombo->setCurrentItem (maleVoices[filename]);
   else
      voiceCombo->setCurrentItem (femaleVoices[filename]);
}

TQString HadifixConfigUI::getVoiceFilename() {
   int curr = voiceCombo->currentItem();

   TQString filename = voiceCombo->text(curr);
   if (defaultVoices.contains(curr))
      filename = defaultVoices[curr];

   return filename;
}

bool HadifixConfigUI::isMaleVoice() {
   int curr = voiceCombo->currentItem();
   TQString filename = getVoiceFilename();

   if (maleVoices.contains(filename))
      return maleVoices[filename] == curr;
   else
      return false;
}

void HadifixConfigUI::changed (const TQString &) {
   emit changed (true);
}
