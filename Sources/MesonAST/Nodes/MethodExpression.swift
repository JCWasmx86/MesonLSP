import SwiftTreeSitter

public class MethodExpression: Expression {
  public let file: MesonSourceFile
  public let obj: Node
  public let id: Node
  public let argumentList: Node?
  public var types: [Type] = []
  public let location: Location

  init(file: MesonSourceFile, node: SwiftTreeSitter.Node) {
    self.file = file
    self.location = Location(node: node)
    self.obj = from_tree(file: file, tree: node.namedChild(at: 0))!
    self.id = from_tree(file: file, tree: node.namedChild(at: 1))!
    self.argumentList =
      node.namedChildCount == 2 ? nil : from_tree(file: file, tree: node.namedChild(at: 3))
  }
  public func visit(visitor: CodeVisitor) { visitor.visitMethodExpression(node: self) }
  public func visitChildren(visitor: CodeVisitor) {
    self.obj.visit(visitor: visitor)
    self.id.visit(visitor: visitor)
    self.argumentList?.visit(visitor: visitor)
  }
}
