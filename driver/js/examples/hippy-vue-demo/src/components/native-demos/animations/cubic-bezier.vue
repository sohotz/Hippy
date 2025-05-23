<template>
  <div>
    <animation
      ref="animationView"
      :playing="playing"
      :actions="loopActions"
      class="loop-green"
    >
      <div class="loop-white">
        <slot />
      </div>
    </animation>
  </div>
</template>

<script>
const horizonAnimation = {
  transform: {
          // 单个 Animation
          translateY: {
            startValue: 200,
            toValue: 0,
            duration: 3000,
            timingFunction: 'cubic-bezier(0.22, 1, 0.36, 1)',
            valueType: 'px', // 动画的开始和结束值的单位类型，默认为 undefined, 可设为 rad、deg、color
          },
        },

};

export default {
  props: {
    playing: Boolean,
    onRef: Function,
  },
  data() {
    return {
      loopActions: horizonAnimation,
    };
  },
  mounted() {
    if (this.$props.onRef) {
      this.$props.onRef(this.$refs.animationView);
    }
  },
};
</script>

<style scoped>
  .loop-green {
    margin-top: 10px;
    justify-content: center;
    align-items: center;
    background-color: #40b883;
    width: 200px;
    height: 80px;
  }

  .loop-white {
    justify-content: center;
    align-items: center;
    background-color: white;
    width: 160px;
    height: 50px;
  }
</style>
