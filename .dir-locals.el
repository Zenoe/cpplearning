;;; dir-locals.el --- Description -*- lexical-binding: t; -*-
;;
;; Copyright (C) 2025 John Doe
;;
;; Author: John Doe <john@doe.com>
;; Maintainer: John Doe <john@doe.com>
;; Created: July 04, 2025
;; Modified: July 04, 2025
;; Version: 0.0.1
;; Keywords: abbrev bib c calendar comm convenience data docs emulations extensions faces files frames games hardware help hypermedia i18n internal languages lisp local maint mail matching mouse multimedia news outlines processes terminals tex tools unix vc wp
;; Homepage: https://github.com/root/dir-locals
;; Package-Requires: ((emacs "24.3"))
;;
;; This file is not part of GNU Emacs.
;;
;;; Commentary:
;;
;;  Description
;;
;;; Code:

;; ((c++-mode . ((flycheck-gcc-args . ("-std=c++17"))
;;               (flycheck-clang-args . ("-std=c++17")))))
((c++-mode . ((lsp-clients-clangd-args . ("--query-driver=/usr/bin/g++,/usr/bin/clang++")))))
;; ((c++-mode . ((lsp-clients-clangd-args . ("--std=c++17")))))
(provide 'dir-locals)
;;; dir-locals.el ends here
