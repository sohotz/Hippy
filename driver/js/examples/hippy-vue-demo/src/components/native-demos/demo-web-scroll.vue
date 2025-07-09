<template>
  <div style="flex: 1; background-color: transparent" @layout="onLayout">
    <!-- TODO: 貌似这里的scrollEventThrottle值越小，越容易复现 -->
    <div style="flex: 1; flex-direction: column; overflow-y: scroll" :scrollEventThrottle="8.3" @scroll="onScroll">
      <!-- ContentView -->
      <div style="flex-direction: column" :style="{ height: layoutHeight + bannerHeight }">
        <div :style="{ height: bannerHeight }">
          <!-- Banner  -->
          <div style="flex: 1; background-color: red" />
          <!-- MaskView -->
          <div
            style="position: absolute; left: 0; top: 0; right: 0; bottom: 0; background-color: #ffffff"
            :style="styleMaskView"
          />
        </div>
        <div :style="{ height: layoutHeight }">
          <ul :nestedScrollTopPriority="'parent'" :nestedScrollBottomPriority="'self'" :bounces="false">
            <template v-for="i in 50">
              <li :key="`li_${i}`">
                <span style="height: 45px" :style="{ backgroundColor: i % 2 === 0 ? 'gray' : 'yellow' }">
                  Item {{ i }}
                </span>
              </li>
            </template>
          </ul>
        </div>
      </div>
    </div>
    <!-- NavBar  -->
    <div
      style="
        position: absolute;
        left: 0;
        top: 0;
        right: 0;
        justify-content: center;
        align-items: center;
        font-size: 16px;
        font-weight: 500;
      "
      :style="styleNavBar"
    >
      <span>这是标题栏</span>
    </div>
  </div>
</template>

<script>
export default {
  name: 'NestedScrollExample',
  data() {
    return {
      bannerHeight: 300,
      navHeight: 104,
      offsetY: 0,
      layoutHeight: 0,
    };
  },
  computed: {
    collapseOffset() {
      return this.bannerHeight - this.navHeight;
    },
    collapsePercent() {
      return this.offsetY / this.collapseOffset;
    },
    styleMaskView() {
      return {
        opacity: this.collapsePercent,
      };
    },
    styleNavBar() {
      const isCollapsed = Math.ceil(this.offsetY) >= this.collapseOffset;
      return {
        paddingTop: 44, // 状态栏高度
        height: this.navHeight,
        color: isCollapsed ? '#000000' : '#00000000',
        backgroundColor: isCollapsed ? '#ffffff' : '#00000000',
      };
    },
  },
  methods: {
    onLayout(payload) {
      this.layoutHeight = payload.height;
    },
    onScroll(payload) {
      this.offsetY = payload.offsetY;
    },
  },
};
</script>