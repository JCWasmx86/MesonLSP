public class Module: AbstractObject {
  public let name: String = "module"
  public var methods: [Method] = []
  public let parent: AbstractObject? = nil

  public init() {}
}
