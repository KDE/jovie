;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;-*-mode:scheme-*-
;;                                                                       ;;
;;                Centre for Speech Technology Research                  ;;
;;                     University of Edinburgh, UK                       ;;
;;                       Copyright (c) 1996,1997                         ;;
;;                        All Rights Reserved.                           ;;
;;                                                                       ;;
;;  Permission is hereby granted, free of charge, to use and distribute  ;;
;;  this software and its documentation without restriction, including   ;;
;;  without limitation the rights to use, copy, modify, merge, publish,  ;;
;;  distribute, sublicense, and/or sell copies of this work, and to      ;;
;;  permit persons to whom this work is furnished to do so, subject to   ;;
;;  the following conditions:                                            ;;
;;   1. The code must retain the above copyright notice, this list of    ;;
;;      conditions and the following disclaimer.                         ;;
;;   2. Any modifications must be clearly marked as such.                ;;
;;   3. Original authors' names are not deleted.                         ;;
;;   4. The authors' names are not used to endorse or promote products   ;;
;;      derived from this software without specific prior written        ;;
;;      permission.                                                      ;;
;;                                                                       ;;
;;  THE UNIVERSITY OF EDINBURGH AND THE CONTRIBUTORS TO THIS WORK        ;;
;;  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ;;
;;  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ;;
;;  SHALL THE UNIVERSITY OF EDINBURGH NOR THE CONTRIBUTORS BE LIABLE     ;;
;;  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ;;
;;  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ;;
;;  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ;;
;;  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ;;
;;  THIS SOFTWARE.                                                       ;;
;;                                                                       ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;           Author:  Alan W Black
;;;           Date:    November 1997
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Adapted from text2wave by Alan Black.  Original copyright listed above.
;;
;; Copyright 2004 by Gary Cramblitt <garycramblitt@comcast.net>
;;
;; This scheme module is used by the Festival Interactive plugin,
;; which is part of KTTSD.  To use,
;;    (load sabletowave.scm)
;; after starting Festival interactively, then to synth text containing
;; SABLE tags to a single wave file.
;;    (ktts_sabletowave "sable text" "filename" volume)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; List of generated intermediate wave files.
(defvar ktts_wavefiles nil)

(define (ktts_save_record_wave utt)
"Saves the waveform and records its so it can be joined into a 
a single waveform at the end."
  (let ((fn (make_tmp_filename)))
    (utt.save.wave utt fn)
    (set! ktts_wavefiles (cons fn ktts_wavefiles))
    utt))

(define (ktts_combine_waves outfile volume)
  "Join all the waves together into the desired output file
and delete the intermediate ones."
  (let ((wholeutt (utt.synth (Utterance Text ""))))
    (mapcar
     (lambda (d) 
       (utt.import.wave wholeutt d t)
       (delete-file d))
     (reverse ktts_wavefiles))
;;    (if ktts_frequency
;;      (utt.wave.resample wholeutt (parse-number ktts_frequency)))
    (if (not (equal? volume "1.0"))
    (begin
      (utt.wave.rescale wholeutt (parse-number volume))))
    (utt.save.wave wholeutt outfile 'riff)
    ))

;;;
;;; Redefine what happens to utterances during text to speech.
;;; Synthesize each utterance and save to a temporary wave file.
;;;
(set! tts_hooks (list utt.synth ktts_save_record_wave))

(define (ktts_sabletowave text filename volume)
  ;; Do the synthesis, which creates multiple wave files.
  (tts_text text 'sable)
  ;; Now put the waveforms together and adjust volume.
  (ktts_combine_waves filename volume)
)
