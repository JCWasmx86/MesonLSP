import Foundation
import MesonAST

public func findDeclaration(node: IdExpression) -> (String, UInt32, UInt32)? {
  if let p = node.parent {
    if let assS = p as? AssignmentStatement, assS.lhs.equals(right: node), assS.op == .equals {
      return makeTuple(node)
    } else {
      return findDeclaration2(name: node.id, node: node, parent: p)
    }
  }
  return nil
}

public func findDeclaration2(name: String, node: Node, parent: Node) -> (String, UInt32, UInt32)? {
  if let sst = parent as? SelectionStatement {
    var block_idx = 0
    var stmt_idx = 0
    var breakLoops = false
    for bb in sst.blocks {
      for b in bb {
        if b.location.startLine <= node.location.startLine
          && b.location.endLine >= node.location.endLine
        {
          // We found our statement
          breakLoops = true
        }
        if breakLoops { break }
        stmt_idx += 1
      }
      if breakLoops { break }
      block_idx += 1
    }
    if block_idx < sst.blocks.count && stmt_idx <= sst.blocks[block_idx].count {
      for idx in 0..<stmt_idx {
        let ridx = (stmt_idx - 1) - idx
        let s = sst.blocks[block_idx][ridx]
        if let assS = s as? AssignmentStatement, let assSLHS = assS.lhs as? IdExpression,
          assSLHS.id == name, assS.op == .equals
        {
          return makeTuple(assS.lhs)
        } else if s is IterationStatement || s is SelectionStatement {
          if let r = searchExtended(name: name, node: s) { return r }
        }
      }
    }
  } else if let bd = parent as? BuildDefinition {
    var stmt_idx = 0
    for b in bd.stmts {
      if (b.location.startLine <= node.location.startLine
        && b.location.endLine >= node.location.endLine) || b.equals(right: node)
      {
        break
      }
      stmt_idx += 1
    }
    if stmt_idx <= bd.stmts.count {
      for idx in 0..<stmt_idx {
        let ridx = (stmt_idx - 1) - idx
        let s = bd.stmts[ridx]
        if let assS = s as? AssignmentStatement, let assSLHS = assS.lhs as? IdExpression,
          assSLHS.id == name, assS.op == .equals
        {
          return makeTuple(assS.lhs)
        } else if s is IterationStatement || s is SelectionStatement {
          if let r = searchExtended(name: name, node: s) { return r }
        }
      }
      if node.parent == nil { return nil }
      return findDeclaration2(name: name, node: parent, parent: node.parent!)
    }
  } else if let its = parent as? IterationStatement {
    var stmt_idx = 0
    for b in its.block {
      if b.location.startLine <= node.location.startLine
        && b.location.endLine >= node.location.endLine
      {
        break
      }
      stmt_idx += 1
    }
    if stmt_idx <= its.block.count {
      for idx in 0..<stmt_idx {
        let ridx = (stmt_idx - 1) - idx
        let s = its.block[ridx]
        if let assS = s as? AssignmentStatement, let assSLHS = assS.lhs as? IdExpression,
          assSLHS.id == name, assS.op == .equals
        {
          return makeTuple(assS.lhs)
        } else if s is IterationStatement || s is SelectionStatement {
          if let r = searchExtended(name: name, node: s) { return r }
        }
      }
    }
    for x in its.ids where x is IdExpression && (x as! IdExpression).id == name {
      return makeTuple(x)
    }
  } else if parent is SourceFile {
    if parent.parent != nil && parent.parent is SubdirCall {
      return findDeclaration2(name: name, node: parent.parent!, parent: parent.parent!.parent!)
    }
  }
  if node.parent != nil && node.parent!.parent != nil {
    return findDeclaration2(name: name, node: node.parent!, parent: node.parent!.parent!)
  }
  return nil
}

func searchExtended(name: String, node: Node) -> (String, UInt32, UInt32)? {
  if let its = node as? IterationStatement {
    for s in its.block.reversed() {
      if let assS = s as? AssignmentStatement, let assSLHS = assS.lhs as? IdExpression,
        assSLHS.id == name, assS.op == .equals
      {
        return makeTuple(assS.lhs)
      } else if s is IterationStatement || s is SelectionStatement {
        if let r = searchExtended(name: name, node: s) { return r }
      }
    }
  } else if let ses = node as? SelectionStatement {
    for block in ses.blocks.reversed() {
      for s in block.reversed() {
        if let assS = s as? AssignmentStatement, let assSLHS = assS.lhs as? IdExpression,
          assSLHS.id == name, assS.op == .equals
        {
          return makeTuple(assS.lhs)
        } else if s is IterationStatement || s is SelectionStatement {
          if let r = searchExtended(name: name, node: s) { return r }
        }
      }
    }
  }
  return nil
}
func makeTuple(_ node: Node) -> (String, UInt32, UInt32) {
  let file = node.file.file
  let line = node.location.startLine
  let column = node.location.startColumn
  return (file, line, column)
}
