# 图着色

## 编译项目

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 运行

```
./GraphColoring  <limit_time> <seed>
```

测试用例请使用标准输入传入程序。

| instance       | k   | Iter_TC | time      |
|----------------|-----|---------|-----------|
| 0              | 5   | Tabucol | 0.001 s   |
| 1              | 17  | Tabucol | 0.114 s   |
| 2              | 44  | Tabucol | 0.004 s   |
| 3              | 8   | Tabucol | 0.032 s   |
| 4 (dsjc250.5)  | 28  | 6000    | 0.478 s   |
| 5 (dsjc500.5)  | 49  | 8000    | 2.028 s   |
|                | 48  | 8000    | 35.434 s  |
| 6 (dsjc1000.5) | 83  | 40000   | 188.3 s   |
| 7 (C2000.5)    | 146 | 140000  | 225.9 min |
| 8              | 409 | 140000  | 20.9 min  |
| 9 (C4000.5)    | 270 | 140000  | 775 min   |

### 测试用例格式说明：

所有算例的节点从 0 开始连续编号.

第一行给出 3 个由空白字符分隔的整数, 分别表示节点数 N, 无向边数 E (有向边数为 2E), 以及参考颜色数 C (相对容易求得可行解,
非最优颜色数).
接下来连续 E 行, 每行包含 2 个由空白字符分隔的整数, 表示一条无向边的两个端点.

例如, 以下算例文件表示节点数为 4, 无向边数为 3, 参考颜色数为 2; 其中:
节点 0 分别与 1, 2, 3 相邻.

```
4 3 2
0 1
0 2
3 0
```

## 参考文献

- L. Moalic and A. Gondran, “Variations on memetic algorithms for graph coloring problems,” Journal of Heuristics, vol.
  24, no. 1, pp. 1–24, 2018, doi: 10.1007/s10732-017-9354-9.