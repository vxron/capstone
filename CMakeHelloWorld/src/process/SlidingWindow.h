/*
==============================================================================
	File: SlidingWindow.h
	Desc: Maintains a rolling, fixed-length, multi-channel EEG window with a configurable
	hop (step). Consumes full chunks from acquisition and emits feature processing/extraction-ready 
	windows at hop boundaries.
	Threading Model: 
	- Called only from the consumer (i.e., decoder thread).
	- No internal locks. Do not call from multiple threads.
	Inputs:
	- fs (Hz), nCh (channels), window length (seconds), hop length (seconds)
	Outputs:
	- full window for downstream preproc/features
==============================================================================
*/