#!/bin/bash

latex index && pdflatex index && latex2html -local_icons index && latex2rtf index && dvips index 
