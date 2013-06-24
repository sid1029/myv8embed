#include <v8.h>
#include "callbacks.h"

using namespace v8;


// Creates a new execution environment containing the built-in
// functions.
v8::Persistent<v8::Context> RegisterFunctions() {
  // Create a template for the global object.
  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
  // Bind the global 'print' function to the C++ Print callback.
  global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));

  return v8::Context::New(NULL, global);
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const char* name) {
	FILE* file = fopen(name, "rb");
	if (file == NULL) return v8::Handle<v8::String>();

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (int i = 0; i < size;) {
		int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
		i += read;
	}
	fclose(file);
	v8::Handle<v8::String> result = v8::String::New(chars, size);
	delete[] chars;
	return result;
}

// Process command line arguments and return script handle
v8::Handle<v8::String> ReadArg(char* str) {
	// Use all other arguments as names of files to load and run.
	v8::Handle<v8::String> source = ReadFile(str);
	if (source.IsEmpty()) {
		fprintf(stderr, "Error reading '%s'\n", str);
	}
	return source;
}

int main(int argc, char* argv[])
{
	// Get the default Isolate created at startup.
	Isolate* isolate = Isolate::GetCurrent();

	// Create a stack-allocated handle scope.
	HandleScope handle_scope(isolate);

	// Create a template for the global object.
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	// Bind the global 'print' function to the C++ Print callback.
	global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));
	// Bind the global 'read' function to the C++ Read callback.
	global->Set(v8::String::New("read"), v8::FunctionTemplate::New(Read));
	// Bind the global 'load' function to the C++ Load callback.
	global->Set(v8::String::New("load"), v8::FunctionTemplate::New(Load));
	// Bind the 'quit' function
	global->Set(v8::String::New("quit"), v8::FunctionTemplate::New(Quit));
	// Bind the 'version' function
	global->Set(v8::String::New("version"), v8::FunctionTemplate::New(Version));

	// Here's how you could create a Persistent handle to the context, if needed.
	Persistent<Context> persistent_context = v8::Context::New(NULL, global);
  
	persistent_context->Enter();

	for (int i = 1; i < argc; i++)
	{
		// Create a string containing the JavaScript source code.
		Handle<String> source = ReadArg(argv[i]);
		// Compile the source code.
		Handle<Script> script = Script::Compile(source);
		// Run the script to get the result.
		Handle<Value> result = script->Run();
		printf("\n\n\n");
	}

	// The persistent handle needs to be eventually disposed.
	persistent_context.Dispose(isolate);
	return 0;
}