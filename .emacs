
(setq default-frame-alist (append (list 
  '(width  . 81)  ; Width set to 81 characters 
  '(height . 50)) ; Height set to 50 lines 
  default-frame-alist)) 

;; Line and column numbers
(setq line-number-mode          t)
(setq column-number-mode        t)

;; Turn off decorations
(tool-bar-mode                  -1)  ; Disable toolbar
(toggle-scroll-bar              -1)  ; Disable scrollbar

(setq inhibit-startup-message   t)   ; Don't want any startup message 
(setq make-backup-files         nil) ; Don't want any backup files 
(setq auto-save-list-file-name  nil) ; Don't want any .saves files 
(setq auto-save-default         nil) ; Don't want any auto saving 

(setq search-highlight           t) ; Highlight search object 
(setq query-replace-highlight    t) ; Highlight query object 
(setq mouse-sel-retain-highlight t) ; Keep mouse high-lightening 

(add-to-list 'load-path "/usr/share/emacs/site-lisp/emacs-goodies-el/color-theme.el")
(add-to-list 'load-path (expand-file-name "~/.emacs.d/lisp"))
(require 'color-theme)
(require 'color-theme-solarized)
(if window-system
(eval-after-load "color-theme-solarized"
  '(progn
     (color-theme-solarized-dark)))
)
(autoload 'csharp-mode "csharp-mode" "Major mode for editing C# code." t)
(add-to-list 'auto-mode-alist '("\\.cs$" . csharp-mode))
(add-to-list 'auto-mode-alist '("\\.module$" . csharp-mode))

(defun my-c-mode-common-hook ()
  (setq c-basic-offset 2)
  (c-set-offset 'substatement-open 0))
(add-hook 'c-mode-common-hook 'my-c-mode-common-hook)
