Sky Module System
=================

This document describes the Sky module system.

Overview
--------

The Sky module system is based on the ``import`` element. In its
most basic form, you import a module as follows:

```html
<import src="path/to/module.sky" />
```

As these ``import`` elements are inserted into a document, the
document's list of outstanding dependencies grows. When an imported
module completes, it is removed from the document's list of
outstanding dependencies.

Before compiling script or inserting an element that is not already
registered, the parser waits until the list of outstanding
dependencies is empty. After the parser has finished parsing, the
document waits until its list of outstanding dependencies is empty
before the module it represents is marked complete.


Module API
----------

Each module consists of one or more libraries. The first library in a
module is the *element tree library*, which imports the sky:core
module and then consists of the following code for a Sky module:

```dart
final Module module = new Module();
```

...and the following code for a Sky application:

```dart
final Module module = new Application();
```

The ``<script>`` elements found in the document create the subsequent
libraries. Each one first imports the ``dart:mirror`` library, then
the ``sky:core`` module, then the first library described above, then
all the modules referenced by ``<import>`` element up to that
``<script>`` element and all the libraries defined by ``<script>``
elements up to that point, interleaved so as to maintain the same
relative order as those elements were first seen by the parser.

When a library imports a module, it actually imports all the libraries
that were declared by that module except the aforementioned element
tree library.

At the end of the ``<script>`` block's source, if it parsed correctly
and completely, the conceptual equivalent of the following code is
appended (but without affecting the library's list of declarations and
without any possibility of it clashing with identifiers described in
the library itself):

```dart
class _ { }
void main() {
  LibraryMirror library = reflectClass(_).owner as LibraryMirror;
  if (library.declarations.containsKey(#init) && library.declarations[#init] is MethodMirror)
    init();
  AutomaticMetadata.runLibrary(library, module);
}
```

Then, that ``main()`` function is called.

TODO(ianh): decide what URL and name we should give the libraries, as
exposed in MirrorSystem.getName(libraryMirror.qualifiedName) etc

The ``Module`` class is defined in ``sky:core`` as follows:

```dart

abstract class AbstractModule extends EventTarget {
  AbstractModule({this.document, this.url});

  final Document document; // O(1)
  // the Document of the module or application

  final String url;

  @nonnull external Future<@nonnull Module> import(String url); // O(Yikes)
  // load and return the URL at the given Module
  // if it's already loaded, the future will resolve immediately
  // if loading fails, the future will have an error

  @nonnull List</*@nonnull*/ Module> getImports(); // O(N)
  // returns the Module objects of all the imported modules

  external registerElement(@nonnull String tagname, @nonnull Type elementClass); // O(1)
  // registers a tag name with the parser
  // only useful during parse time
  // verify that tagname isn't null or empty
  // verify that elementClass is the Type of a class that extends Element (directly or indirectly, but not via "implements" or "with")
  // (see the @tagname code for an example of how to verify that from dart)
  // verify that there's not already a class registered for this tag name
  // if there is, then mark this tagname is broken, so that it acts as if it's not registered in the parser,
  // and, if this is the first time it was marked broken, log a console message regarding the issue
  // (mention the tag name but not the classes, so that it's not observable that this currently happens out of order)
}

class Module : AbstractModule {
  constructor (Application application, Document document, String url); // O(1)
  readonly attribute Application application; // O(1)
}

class Application : AbstractModule {
  constructor (Document document, GestureManager gestureManager, String url); // O(1)
  attribute String title; // O(1)
  readonly attribute GestureManager gestureManager;
}
```

 
Naming modules
--------------

The ``as`` attribute on the ``import`` element binds a name to the
imported module:

```html
<import src="path/to/chocolate.sky" as="chocolate" />
```

The parser executes the contents of script elements inside a module as
if they were executed as follow:

```javascript
(new Function(name_1, ..., name_n, module, source_code)).call(
  value_1, ..., value_n, source_module);
```

Where ``name_1`` through ``name_n`` are the names bound to the
various named imports in the script element's document,
``source_code`` is the text content of the script element,
``source_module`` is the ``Module`` object of the script element's
module, and ``value_1`` through ``value_n`` are the values
exported by the various named imports in the script element's
document.

When an import fails to load, the ``as`` name for the import gets
bound to ``undefined``.
