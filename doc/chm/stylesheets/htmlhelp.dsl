<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY dbstyle SYSTEM "docbook.dsl" CDATA DSSSL>
]>

<style-sheet>

<style-specification use="docbook">
<style-specification-body> 

;; These are some customizations to the Modular DocBook Stylesheets to produce
;; compiled HTML help files.
;;
;; José Fonseca



;; Don't generate Table of Contents

(define ($generate-chapter-toc$) #f)
(define ($generate-qandaset-toc$) #f)

(define %generate-article-toc% #f)
(define %generate-book-toc% #f)
(define %generate-part-toc% #f)
(define %generate-reference-toc% #f)
(define %generate-set-toc% #f)

;; Stylesheet customization (taken from ldp.dsl)

(define %html-ext%
  ;; when producing HTML files, use this extension
  ".html")

(define (chunk-skip-first-element-list)
  ;; forces the Table of Contents on separate page
  '())

(define %root-filename%
  ;; The filename of the root HTML document (e.g, "index").
  "index")

(define %use-id-as-filename%
  ;; Use ID attributes as name for component HTML files?
  #t)


;; Element generation (based on Norman Walsh's htmlhelp.dsl)

(define (extract-gi args)
  (let ((gi (member gi: args)))
    (if gi
	(car (cdr gi))
	"")))

(define (extract-node args)
  (let ((node (member node: args)))
    (if node
	(car (cdr node))
	#f)))

(define (extract-attributes args)
  (let ((attr (member attributes: args)))
    (if attr
	(car (cdr attr))
	'())))

(define (extract-sosofos args)
  (let loop ((l args) (results '()))
    (if (null? l)
	results
	(if (not (keyword? (car l)))
	    (loop (cdr l) (append results (list (car l))))
	    (loop (cdr (cdr l)) results)))))

(define (make-element #!rest args)
  ;; Args _MUST_ be '( gi: "gi" attributes: '() sosofo...) where sosofo
  ;; is optional.
  (let* ((node       (if (extract-node args)
			 (extract-node args)
			 (current-node)))
	 (giname     (extract-gi args))
	 (attr       (extract-attributes args))
	 (sosofo     (extract-sosofos args)))
    (sosofo-append
      (make formatting-instruction data: (string-append "<" giname))
      (if (null? attr)
	  (empty-sosofo)
	  (let loop ((a attr))
	    (if (null? a)
		(empty-sosofo)
		(make sequence
		  (let* ((attrlist (car a))
			 (name (car attrlist))
			 (value (car (cdr attrlist))))
		    (make formatting-instruction 
		      data: (string-append " " name "=\"" (if value value "whatthe") "\"")))
		  (loop (cdr a))))))
      
      (make formatting-instruction data: ">")
      (htmlnewline)

      (if sosofo
	  (apply sosofo-append sosofo)
	  (current-node))

      (make formatting-instruction data: (string-append "</" giname ">"))
      (htmlnewline ))))

(define (make-empty-element #!rest args)
  ;; Args _MUST_ be '( gi: "gi" attributes: '() sosofo)
  (let* ((giname (extract-gi args))
	 (attributes (extract-attributes args))
	 (attr attributes))
    (sosofo-append
      (make formatting-instruction data: (string-append "<" giname))
      (if (null? attr)
	  (empty-sosofo)
	  (let loop ((a attr))
	    (if (null? a)
		(empty-sosofo)
		(make sequence
		  (make formatting-instruction 
		    data: (string-append " " 
					 (car (car a)) 
					 "=\"" 
					 (car (cdr (car a)))
					 "\""))
		  (loop (cdr a))))))

      (make formatting-instruction data: ">")
      (htmlnewline))))

;; Table of Contents generation (based on dbautoc.dsl, v1.76)

(define (htmlhelp-toc-depth)
  10)

(define (build-htmlhelp-toc nd depth #!optional (first? #t))
  (let ((toclist (toc-list-filter 
		  (node-list-filter-by-gi (children nd)
					  (append (division-element-list)
						  (component-element-list)
						  (section-element-list))))))
    (if (or (<= depth 0) 
	    (and (node-list-empty? toclist) #f))
      (empty-sosofo)
      (make sequence
        (make-element gi: "LI"
	  (make-element gi: "OBJECT"
	    attributes: '(("type" "text/sitemap"))
	    (make-empty-element gi: "param"
	      attributes: (list (list "name" "Name")
			  (list "value" (element-title-string nd))))
	    (make-empty-element gi: "param"
	      attributes: (list (list "name" "Local")
			  (list "value" (href-to nd))))))
	  (make-element gi: "UL"
	    (let loop ((nl toclist))
	      (if (node-list-empty? nl)
		  (empty-sosofo)
		  (sosofo-append
		    (build-htmlhelp-toc (node-list-first nl) 
			       (- depth 1) #f)
		    (loop (node-list-rest nl))))))))))

(define (make-htmlhelp-contents)
  (make sequence
    (make document-type
      name: "HTML"
      public-id: "-//IETF//DTD HTML//EN")
    (make-element gi: "HTML"
      (make-element gi: "BODY"
	(make-element gi: "UL"
	  (build-htmlhelp-toc (sgml-root-element) (htmlhelp-toc-depth)))))))

(define (make-htmlhelp-contents2)
  (make sequence
    (make document-type
      name: "HTML"
      public-id: "-//IETF//DTD HTML//EN")
    (make element gi: "HTML"
      (make element gi: "BODY"
	(make element gi: "UL"
	  (make element gi: "LI"
	    (make element gi: "OBJECT"
	      attributes: '(("type" "text/sitemap"))
	      (make empty-element gi: "param"
		attributes: (list (list "name" "Name")
			    (list "value" (element-title-string (sgml-root-element)))))
	      (make empty-element gi: "param"
		attributes: (list (list "name" "Local")
			    (list "value" (html-base-filename (sgml-root-element))))))
	    (make element gi: "UL"
	      (build-htmlhelp-toc (sgml-root-element) (htmlhelp-toc-depth)))))))))

;; Index processing (based on dbindes.dsl, v1.76)

(define (htmlhelpindexattr attr)
  (if (attribute-string (normalize attr))
      (make sequence
	(make formatting-instruction data: attr)
	(make formatting-instruction data: " ")
	(make formatting-instruction data: (attribute-string 
					    (normalize attr)))
	(htmlnewline))
      (empty-sosofo)))

(define (htmlhelpindexterm)
  (let* ((attr    (gi (current-node)))
	 (content (data (current-node)))
	 (string  (string-replace content "&#13;" " "))
	 (sortas  (attribute-string (normalize "sortas"))))
    (make sequence
      (make formatting-instruction data: string))))

(define (htmlhelpindexzone zone)
  (let loop ((idlist (split zone)))
    (if (null? idlist)
	(empty-sosofo)
	(make sequence
	  (htmlhelpindexzone1 (car idlist))
	  (loop (cdr idlist))))))

(define (htmlhelpindexzone1 id)
  (let* ((target (ancestor-member (element-with-id id)
				  (append (book-element-list)
					  (division-element-list)
					  (component-element-list)
					  (section-element-list))))
	 (title  (string-replace (element-title-string target) "&#13;" " ")))
    (make sequence
      (make formatting-instruction data: "ZONE ")
      (make formatting-instruction data: (href-to target))
      (htmlnewline)

      (make formatting-instruction data: "TITLE ")
      (make formatting-instruction data: title)
      (htmlnewline))))

(mode htmlhelpindex
  ;; this mode is really just a hack to get at the root element
  (root (process-children))

  (default 
    (if (node-list=? (current-node) (sgml-root-element))
	  (process-node-list (select-elements 
			      (descendants (current-node))
			      (normalize "indexterm")))
	(empty-sosofo)))

  (element indexterm
      (make sequence
	(make formatting-instruction data: "\less-than-sign;LI")

	(make formatting-instruction data: " name=\"")

;;	(htmlhelpindexattr "scope")
;;	(htmlhelpindexattr "significance")
;;	(htmlhelpindexattr "class")
;;	(htmlhelpindexattr "id")
;;	(htmlhelpindexattr "startref")
	
;;	(if (attribute-string (normalize "zone"))
;;	    (htmlhelpindexzone (attribute-string (normalize "zone")))
;;	    (empty-sosofo))

	(process-children)

	(make formatting-instruction data: "\"")

	(make formatting-instruction data: (string-append " link=\"" (href-to (current-node)) "\""))

	(make formatting-instruction data: "/\greater-than-sign;&#13;")))
		    
  (element primary
    (htmlhelpindexterm))

;;  (element secondary
;;    (htmlhelpindexterm))

;;  (element tertiary
;;    (htmlhelpindexterm))

;;  (element see
;;    (htmlhelpindexterm))

;;  (element seealso
;;    (htmlhelpindexterm))
)

(define (make-htmlhelp-index)
  (make sequence
    (make document-type
      name: "HTML"
      public-id: "-//IETF//DTD HTML//EN")
    (make element gi: "HTML"
      (make element gi: "BODY"
	(with-mode htmlhelpindex
          (process-children))))))

;; Generates a HTML help project

(define (make-htmlhelp-project)
  (let* ((rootgi (case-fold-down (gi (sgml-root-element))))
	 (chm (string-append rootgi ".chm"))
	 (hhp (string-append rootgi ".hhp"))
	 (hhc (string-append rootgi ".hhc"))
	 (hhk (string-append rootgi ".hhk"))
	 (top (html-base-filename (sgml-root-element))))
    (make sequence
      (make entity
	system-id: hhp
	(make sequence
	  (make formatting-instruction data: "[OPTIONS]&#13;")
	  (make formatting-instruction data: "Auto Index=Yes&#13;")
	  (make formatting-instruction data: "Compatibility=1.1&#13;")
	  (make formatting-instruction data: (string-append "Compiled file=" chm "&#13;"))
	  (make formatting-instruction data: (string-append "Contents file=" hhc "&#13;"))
	  (make formatting-instruction data: "Default Window=Default&#13;")
	  (make formatting-instruction data: (string-append "Default topic=" top "&#13;"))
	  (make formatting-instruction data: "Full-text search=Yes&#13;")
	  (make formatting-instruction data: (string-append "Index file=" hhk "&#13;"))
	  (make formatting-instruction data: "Language=0x409 English (United States)&#13;")
	  (make formatting-instruction data: "&#13;")
	  (make formatting-instruction data: "[WINDOWS]&#13;")
	  (make formatting-instruction data: (string-append "Default=," hhc "," hhk "," top "," top ",,,,,0x22520,,0x384e,,,,,,,,0&#13;"))
	  (make formatting-instruction data: "&#13;")
	  (make formatting-instruction data: "[FILES]&#13;")
	  (let loop ((node (current-node)))
	    (if (node-list-empty? node)
		(empty-sosofo)
		(make sequence
		  (make formatting-instruction data: (string-append (html-file node) "&#13;"))
		  (loop (next-chunk-element node)))))))
      (make entity
	system-id: hhc
	  (make-htmlhelp-contents))
      (make entity
	system-id: hhk
	  (make-htmlhelp-index)))))

;; Overrides the root node definition (taken from docbook.dsl, v1.76)

(root
 (make sequence
   (process-children)
   (with-mode manifest
     (process-children))
   (if html-index
       (with-mode htmlindex
	 (process-children))
       (empty-sosofo))
   (make-htmlhelp-project)))

</style-specification-body>
</style-specification>

<external-specification id="docbook" document="dbstyle">

</style-sheet>
