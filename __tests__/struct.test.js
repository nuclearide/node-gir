const { load } = require('../');

const Gdk = load('Gdk');
const GLib = load('GLib');
const GIRepository = load('GIRepository');
const repo = new GIRepository.Repository();
repo.require('Gtk', '3.0', 0);

describe('struct', () => {
  describe('constructors', () => {
    test('can be constructed using properties object', () => {
      const rectangle = new Gdk.Rectangle({
        width: 10,
        height: 20,
        x: 0,
        y: -5
      });
      expect(typeof (rectangle)).toEqual('object');
      expect(rectangle.width).toEqual(10);
      expect(rectangle.height).toEqual(20);
      expect(rectangle.x).toEqual(0);
      expect(rectangle.y).toEqual(-5);
    });

    it('can be constructed with default constructor', () => {
      const loop = new GLib.MainLoop(null, false);
      expect(typeof (loop)).toEqual('object');
      const running = loop.isRunning();
      expect(running).toBe(false);
    });

    it('can be constructed without any arguments', () => {
      const context = new GLib.MainContext();
      const rectangle = new Gdk.Rectangle();
      expect(typeof (context)).toEqual('object');
      expect(typeof (rectangle)).toEqual('object');
    });
  });

  test('can be returned from native functions', () => {
    const info = repo.findByName('Gtk', 'Button');
    expect(typeof (info)).toEqual('object');
    expect(info.getNamespace()).toEqual('Gtk');
    expect(info.getName()).toEqual('Button');
  });

  test('can call functions with structs', () => {
    const info = repo.findByName('Gtk', 'Button');
    const parentInfo = GIRepository.objectInfoGetParent(info);
    expect(parentInfo).not.toBe(null);
    expect(parentInfo).not.toBe(undefined);
    expect(parentInfo.getNamespace()).toEqual('Gtk');
    expect(parentInfo.getName()).toEqual('Bin');
  });

  test('can call methods on structs', () => {
    const buttonInfo = repo.findByName('Gtk', 'Button');
    const propertyInfo = GIRepository.objectInfoGetProperty(buttonInfo, 0);
    const typeInfo = GIRepository.propertyInfoGetType(propertyInfo);
    const typeTag = GIRepository.typeInfoGetTag(typeInfo);
    const typeTagName = GIRepository.typeTagToString(typeTag);
    expect(typeTagName).not.toEqual(null);
    expect(typeTagName).not.toEqual(undefined);
    expect(typeof (typeTagName)).toEqual('string');
  });

  test('"a instanceof b" (and vice versa) should be true for different instances of the same struct', () => {
    const structA = repo.findByName('Gtk', 'Button');
    const structB = repo.findByName('Gtk', 'Button');
    expect(structA instanceof structB.constructor).toBe(true);
    expect(structB instanceof structA.constructor).toBe(true);
  });
});
