#pragma once

/*
  Assumes the front and back of the string end on valid character
  alignments. Use utf8Sync(Front/Back) if you're unsure.
 */
str_t utf8DropChars(str_t s, ptrdiff_t count);

/* Will abort() if head isn't the start of a utf8 character */
ptrdiff_t utf8BytesNeeded(char head);
