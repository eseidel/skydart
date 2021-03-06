Sky Script Language
===================

The Sky script language is Dart.

The way that Sky integrates the module system with its script language
is described in [modules.md](modules.md).

When an method defined as ``external`` receives an argument, it must
type-check it, and, if the argument's value is the wrong type, then it
must throw an ArgumentError as follows:

   throw new ArgumentError(value, name: name);

...where "name" is the name of the argument.

Further, if the type of the argument is annotated with ``@nonnull``,
then the method must additionally throw if the value is of type Null,
as follows:

   throw new ArgumentError.notNull(name);

The ``@nonnull`` annotation is defined as follows:

```dart
const nonnull = const Object();
```

The ``@nonnull`` annotation does nothing in code not marked
``external``, but it has been included anyway for documentation
purposes. It indicates places where providing a null is a contract
violation and that results are therefore likely to be poor.

The following definitions are exposed in ``sky:core``:

```dart
abstract class AutomaticMetadata {
  const AutomaticMetadata();
  void init(DeclarationMirror target, Module module);

  static void runLibrary(LibraryMirror library, Module module) {
    library.declarations.values.toList()..sort((DeclarationMirror a, DeclarationMirror b) {
      bool aHasLocation;
      try {
        aHasLocation = a.location != null;
      } catch(e) {
        aHasLocation = false;
      }
      bool bHasLocation;
      try {
        bHasLocation = b.location != null;
      } catch(e) {
        bHasLocation = false;
      }
      if (!aHasLocation)
        return bHasLocation ? 1 : 0;
      if (!bHasLocation)
        return -1;
      if (a.location.sourceUri != b.location.sourceUri)
        return a.location.sourceUri.toString().compareTo(b.location.sourceUri.toString());
      if (a.location.line != b.location.line)
        return a.location.line - b.location.line;
      return a.location.column - b.location.column;
    })
    ..forEach((DeclarationMirror d) {
      d.metadata.forEach((InstanceMirror i) {
        if (i.reflectee is AutomaticMetadata)
          i.reflectee.run(d, module);
      });
    });
  }
}

class AutomaticFunction extends AutomaticMetadata {
  const AutomaticFunction();
  void init(DeclarationMirror target, Module module) {
    assert(target is MethodMirror);
    MethodMirror f = target as MethodMirror;
    assert(!f.isAbstract);
    assert(f.isRegularMethod);
    assert(f.isTopLevel);
    assert(f.isStatic);
    assert(f.parameters.length == 0);
    assert(f.returnType == currentMirrorSystem().voidType);
    (f.owner as LibraryMirror).invoke(f.simpleName, []);
  }
}
const autorun = const AutomaticFunction();
```

Extensions
----------

The following as-yet unimplemented features of the Dart language are
assumed to exist:

* It is assumed that a subclass can define a constructor by reference
  to a superclass' constructor, wherein the subclass' constructor has
  the same arguments as the superclass' constructor and does nothing
  but invoke that superclass' constructor with the same arguments. The
  syntax for defining this is, within the class body for a class
  called ClassName:

```dart
     ClassName = SuperclassName;
     ClassName.namedConstructor = SuperclassName.otherNamedConstructor;
```

* It is assumed that the standard library includes something that
  matches this pattern:

```dart
class DispatcherController<T> {
  Dispatcher<T> _dispatcher;
  Dispatcher<T> get dispatcher => _dispatcher;

  void add(T data) {
    // ...
  }
}
typedef bool Filter<T>(T t);
typedef void Handler<T>(T t);
class Dispatcher<T> {
  Dispatcher<T> where(Filter<T> filter) { /*...*/ return this; }
  void listen(Handler<T> handler) { /* ... */ }
}
class ExceptionListException<T> extends Exception with IterableMixin<T> {
  List<T> _exceptions;
  void add(T exception) {
    if (_exceptions == null)
      _exceptions = new List<T>();
    _exceptions.add(exception);
  }
  int get length => _exceptions == null ? 0 : _exceptions.length;
  Iterator<T> iterator() => _exceptions.iterator();
}
```
