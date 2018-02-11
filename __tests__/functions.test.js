const { load, Gtk } = require('../');

const GObject = load('GObject');
const GdkPixbuf = load('GdkPixbuf');
const GIRepository = load('GIRepository');

const pixbuf = new GdkPixbuf.Pixbuf();

describe('functions', () => {
  describe('functions can be called', () => {
    test('static methods should be callable', () => {
      const button = Gtk.Button.newWithLabel('my button'); // static method
      expect(button).not.toBe(undefined);
      expect(button.label).toEqual('my button');
    });

    test('object methods should be callable', () => {
      const button = new Gtk.Button();
      button.setLabel('my label'); // object method
      expect(button.label).toEqual('my label');
    });
  });

  describe('functions can return values', () => {
    test('functions can return a: number', () => {
      const intValue = GObject.typeFromName('GtkWindow');
      expect(typeof intValue).toEqual('number');
    });

    test('functions can return a: string', () => {
      const window = new Gtk.Window();
      window.title = 'Lancelot';
      expect(window.title).toEqual('Lancelot');
      expect(typeof window.title).toEqual('string');
    });

    test('functions can return a: boolean', () => {
      const window = new Gtk.Window();
      const decorated = window.getDecorated();
      expect(typeof decorated).toEqual('boolean');
    });

    test('functions can return: null', () => {
      const window = new Gtk.Window();
      window.setIcon(null);
      expect(window.getIcon()).toBeNull();
    });

    test('functions can return an: object', () => {
      const window = new Gtk.Window();
      window.icon = pixbuf;
      expect(typeof window.icon).toEqual('object');
    });

    test('void functions return undefined', () => {
      const window = new Gtk.Window();
      const voidValue = window.resize(10, 10);
      expect(voidValue).toBeUndefined();
    });

    // FIXME: this is segfaulting on some machines but not in CI
    // it seg faults in the g_type_children() native c function
    // so i think it's a bug with a particular version of
    // libgobject-introspection not the bindings!
    // test('functions can return an: array', function () {
    //   const gtype = GObject.typeFromName('GtkWindow');
    //   const gtypeArray = GObject.typeInterfaces(gtype);
    //   expect(Array.isArray(gtypeArray)).toBe(true);
    //   expect(typeof(gtypeArray.length)).toEqual('number');
    //   expect(gtypeArray.length).not.toEqual(0);
    // });

    test('functions can return UTF8 (string) arrays', () => {
      const repository = new GIRepository.Repository();
      repository.require('Gtk', '3.0', 0);
      const result = repository.getDependencies('Gtk');
      expect(Array.isArray(result)).toBe(true);
      expect(result.length).not.toEqual(0);
      result.forEach((element) => {
        expect(typeof element).toEqual('string');
      });
    });

    test('functions with out arguments should return an array', () => {
      const button = new Gtk.Button();
      const result = button.getPreferredSize();
      expect(Array.isArray(result)).toBe(true);
      expect(result.length).toEqual(2);
    });
  });

  describe('functions throw errors', () => {
    test('TypeError is thrown when passing an invalid argument type', () => {
      const window = new Gtk.Window();
      expect(() => window.setTitle(['an', 'array'])).toThrow();
    });
  });
});
