# Assignment_3_CSE434

This is a exercise in using socket programming


in the TCP part when we want to send the buffer we need to use sizeof instead of strlen because strlen stopps a the first /0 which is part of our file size length, not the actual end of the header

link to the assignment Google Doc at 
https://docs.google.com/document/d/1qhCTIDbJLEyaCwG3jlabN9hrZRLSutA_EafIamfLPnw/edit