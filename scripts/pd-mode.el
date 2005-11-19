;;; pd-mode.el --- major mode for editing Pd configuration files

;; Author: Hans-Christoph Steiner <hans@at.or.at>
;; Keywords:    languages, faces
;; Last edit: 
;; Version: 1.0.1 

;; This file is an add-on for XEmacs or GNU Emacs (not tested with the latter).
;;
;; It is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.x
;;
;; It is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with your copy of Emacs; see the file COPYING.  If not, write
;; to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.

;;; Commentary:
;;
;; There isn't really much to say.  The list of keywords was derived from
;; the Pd there may be some errors or omissions.
;;
;; There are currently no local keybindings defined, but the hooks are
;; there in the event that anyone gets around to adding any.
;;
;; To enable automatic selection of this mode when appropriate files are
;; visited, add the following to your favourite site or personal Emacs
;; configuration file:
;;
;;   (autoload 'pd-mode "pd-mode" "autoloaded" t)
;;   (add-to-list 'auto-mode-alist '("\\.max$" . pd-mode))
;;   (add-to-list 'auto-mode-alist '("\\.pd$"  . pd-mode))
;;
 
;;; Code:

;; Requires
(require 'regexp-opt)


;; Variables
(defvar pd-mode-map nil
  "Keymap used for Pd file buffers")

(defvar pd-mode-syntax-table nil
  "Pd file mode syntax table")

(defvar pd-mode-hook nil
  "*List of hook functions run by `pd-mode' (see `run-hooks')")


;; Font lock
(defconst pd-font-lock-keywords
  (purecopy
   (list
    (list (concat                                       ; object types
           "^#X \\("
           (regexp-opt '("connect" "floatatom" "msg" "obj" "scalar" "struct" 
								 "symbolatom" "text"))
           "\\) ")
      1 'font-lock-type-face)

    (list (concat                                       ; object types
           "\\(^#[NX] "
           (regexp-opt '("canvas" "restore"))
           "\\) ")
      1 'font-lock-warning-face)

	 ; connect numbers
    (list "^#X connect \\([0-9]+\\) [0-9]+ [0-9]+ [0-9]+"
          1 'font-lock-variable-name-face)
    (list "^#X connect [0-9]+ \\([0-9]+\\) [0-9]+ [0-9]+"
          1 'font-lock-function-name-face)
    (list "^#X connect [0-9]+ [0-9]+ \\([0-9]+\\) [0-9]+"
          1 'font-lock-variable-name-face)
    (list "^#X connect [0-9]+ [0-9]+ [0-9]+ \\([0-9]+\\)"
          1 'font-lock-function-name-face)

	 ; x y coords
    (list "^#X [a-z]+ \\([0-9]+ [0-9]+\\) "
          1 'font-lock-reference-face)

; terminating semi-colon
    (list ";$" 0 'font-lock-warning-face t)

    (list "\\(^#X\\)" 0 'font-lock-builtin-face t)

;    (list "\\(^#N\\)" 0 'font-lock-constant-face t)

;    (list "^#N.*$" 0 'font-lock-comment-face t)

    (list "^#X obj [0-9]+ [0-9]+ \\([a-zA-Z0-9+*._-]+\\)[ ;]" 
			 1 'font-lock-constant-face t)

    (list "^#X msg [0-9]+ [0-9]+ \\([a-zA-Z0-9+*._-]+\\);" 
			 1 'font-lock-variable-name-face)

;    (list "^#X text [0-9]+ [0-9]+ \\(.*\\);" 
;			 1 'font-lock-comment-face)
	 ))
  "Expressions to highlight in Pd config buffers.")

(put 'pd-mode 'font-lock-defaults '(pd-font-lock-keywords nil t
                                                                  ((?_ . "w")
                                                                   (?- . "w"))))
;; Syntax table
(if pd-mode-syntax-table
    nil
  (setq pd-mode-syntax-table (copy-syntax-table nil))
  (modify-syntax-entry ?_   "_"     pd-mode-syntax-table)
  (modify-syntax-entry ?-   "_"     pd-mode-syntax-table)
  (modify-syntax-entry ?\(  "(\)"   pd-mode-syntax-table)
  (modify-syntax-entry ?\)  ")\("   pd-mode-syntax-table)
  (modify-syntax-entry ?\<  "(\>"   pd-mode-syntax-table)
  (modify-syntax-entry ?\>  ")\<"   pd-mode-syntax-table)
  (modify-syntax-entry ?\"   "\""   pd-mode-syntax-table))


;;;###autoload
(defun pd-mode ()
  "Major mode for editing Pd configuration files.

\\{pd-mode-map}

\\[pd-mode] runs the hook `pd-mode-hook'."
  (interactive)
  (kill-all-local-variables)
  (use-local-map pd-mode-map)
  (set-syntax-table pd-mode-syntax-table)

  (make-local-variable 'comment-start)
  (setq comment-start "#N ")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "#\\W*")
  (make-local-variable 'comment-column)
  (setq comment-column 48)

  ;;font-lock stuff (may not be initially necessary for XEmacs)
  (font-lock-mode 1)
  (setq font-lock-keywords pd-font-lock-keywords)
  (font-lock-fontify-buffer)
  
  (setq mode-name "Pd")
  (setq major-mode 'pd-mode)

  (run-hooks 'pd-mode-hook))


;; Provides
(provide 'pd-mode)

;;; pd-mode.el ends here
