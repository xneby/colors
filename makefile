OBJECTS = colors all-xterm

.PHONY: first clean

first: $(OBJECTS)

clean:
	$(RM) $(OBJECTS)

