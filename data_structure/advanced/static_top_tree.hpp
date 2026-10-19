#ifndef STATIC_TOP_TREE_HPP
#define STATIC_TOP_TREE_HPP

#include <cassert>
#include <utility>
#include <vector>
#include "../../components/static_top_tree_structure.hpp"

template <class DP>
class StaticTopTree {
	using Structure = static_top_tree_detail::Structure;
	using PathValue = typename DP::Path;
	using PointValue = typename DP::Point;
	struct Link { int index = -1; Structure::Kind kind = Structure::Vertex; bool right = false; };
	struct VertexNode { typename DP::Vertex info; PointValue light; Link up; };
	struct PathNode { PathValue child[2]; Link up; };
	struct PointNode { PointValue child[2]; Link up; };
	std::vector<VertexNode> vertices_;
	std::vector<PathNode> paths_;
	std::vector<PointNode> points_;
	std::vector<int> parent_, edge_child_;
	PathValue all_ = DP::path_identity();
	int root_ = -1;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }

  public:
	using Vertex = typename DP::Vertex;
	using Path = typename DP::Path;
	using Point = typename DP::Point;
	StaticTopTree() = default;
	StaticTopTree(const HLD& tree, std::vector<Vertex> values) { build(tree, std::move(values)); }
	void build(const HLD& tree, std::vector<Vertex> values) {
		assert(int(values.size()) == tree.n);
		Structure structure(tree);
		int n = tree.n;
		vertices_.clear(), vertices_.reserve(n);
		for (auto& value : values) vertices_.push_back({std::move(value), DP::point_identity(), {}});
		paths_.assign(structure.paths - n, PathNode{{DP::path_identity(), DP::path_identity()}, {}});
		points_.assign(structure.points, PointNode{{DP::point_identity(), DP::point_identity()}, {}});
		std::vector<Path> paths(structure.paths, DP::path_identity());
		std::vector<Point> points(structure.points, DP::point_identity());
		auto point = [&](int id) {
			if (id == -1) return DP::point_identity();
			const auto& node = structure.nodes[id];
			return node.kind == Structure::Rake ? points[node.index] : DP::add_edge(paths[node.index]);
		};
		for (int id : structure.order) {
			const auto& node = structure.nodes[id];
			Link up;
			if (node.parent != -1) {
				const auto& p = structure.nodes[node.parent];
				up = {p.kind == Structure::Compress ? p.index - n : p.index, p.kind, p.right == id};
			}
			if (node.kind == Structure::Vertex) {
				auto& v = vertices_[id];
				v.light = point(node.left), v.up = up;
				paths[id] = DP::add_vertex(v.light, v.info);
			} else if (node.kind == Structure::Compress) {
				auto& v = paths_[node.index - n];
				v = {{paths[structure.nodes[node.left].index], paths[structure.nodes[node.right].index]}, up};
				paths[node.index] = DP::compress(v.child[0], v.child[1]);
			} else {
				auto& v = points_[node.index];
				v = {{point(node.left), point(node.right)}, up};
				points[node.index] = DP::rake(v.child[0], v.child[1]);
			}
		}
		all_ = n ? paths[structure.nodes[structure.root].index] : DP::path_identity();
		root_ = structure.root_vertex;
		parent_ = std::move(structure.parent), edge_child_ = std::move(structure.edge_child);
	}
	int size() const { return int(vertices_.size()); }
	bool empty() const { return vertices_.empty(); }
	int root() const { return root_; }
	int parent(int v) const { check(v); return parent_[v]; }
	int edge_vertex(int e) const {
		assert(0 <= e && e < int(edge_child_.size()));
		return edge_child_[e];
	}
	const Vertex& get(int v) const { check(v); return vertices_[v].info; }
	void set(int v, const Vertex& value) {
		check(v);
		auto& vertex = vertices_[v];
		vertex.info = value;
		Path path = DP::add_vertex(vertex.light, vertex.info);
		Link up = vertex.up;
		while (up.index != -1) {
			while (up.index != -1 && up.kind == Structure::Compress) {
				auto& node = paths_[up.index];
				node.child[up.right] = path;
				path = DP::compress(node.child[0], node.child[1]), up = node.up;
			}
			if (up.index == -1) break;
			Point point = DP::add_edge(path);
			while (up.kind == Structure::Rake) {
				auto& node = points_[up.index];
				node.child[up.right] = point;
				point = DP::rake(node.child[0], node.child[1]), up = node.up;
			}
			auto& node = vertices_[up.index];
			node.light = point;
			path = DP::add_vertex(point, node.info), up = node.up;
		}
		all_ = path;
	}
	Path all_prod() const { return all_; }
	Point dp() const { return DP::add_edge(all_); }
	Path subtree_prod(int v) const {
		check(v);
		const auto& vertex = vertices_[v];
		Path result = DP::add_vertex(vertex.light, vertex.info);
		Link up = vertex.up;
		while (up.index != -1 && up.kind == Structure::Compress) {
			const auto& node = paths_[up.index];
			if (!up.right) result = DP::compress(result, node.child[1]);
			up = node.up;
		}
		return result;
	}
	Path prod(int v) const {
		check(v);
		Point light = vertices_[v].light;
		Path result = DP::path_identity();
		Link up = vertices_[v].up;
		while (true) {
			Path above = DP::path_identity(), below = DP::path_identity();
			while (up.index != -1 && up.kind == Structure::Compress) {
				const auto& node = paths_[up.index];
				if (!up.right) below = DP::compress(below, node.child[1]);
				else above = DP::compress(node.child[0], above);
				up = node.up;
			}
			light = DP::rake(light, DP::add_edge(below));
			result = DP::compress(above, DP::compress(DP::add_vertex(light, vertices_[v].info), result));
			if (up.index == -1) return result;
			light = DP::point_identity();
			while (up.kind == Structure::Rake) {
				const auto& node = points_[up.index];
				if (!up.right) light = DP::rake(light, node.child[1]);
				else light = DP::rake(node.child[0], light);
				up = node.up;
			}
			v = up.index, up = vertices_[v].up;
		}
	}
};

#endif
