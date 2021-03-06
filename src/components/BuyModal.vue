<template>
  <form action="">
    <div class="modal-card" style="width: auto">
      <header class="modal-card-head">
        <p class="modal-card-title">{{$t('buy_modal_title')}} {{celebBaseList[priceInfo.id].name}}</p>
      </header>
      <section class="modal-card-body">
        <b-field :label="$t('buy_modal_label')">
          <b-field>
            <b-input
              v-bind:value="(getPrice() / 10000).toFixed(4)"
              readonly>
            </b-input>
            <p class="control">
              <a class="button is-static">
                ALLIN
              </a>
            </p>
          </b-field>
        </b-field>
      </section>
      <footer class="modal-card-foot">
        <button class="button is-rounded is-primary" @click="buy">{{$t('buy_modal_pay')}}</button>
        <button class="button is-rounded" type="button" @click="$parent.close()">{{$t('buy_modal_cancel')}}</button>
      </footer>
    </div>
  </form>
</template>

<script>
import { mapState, mapGetters } from "vuex";
import * as util from "../blockchain/util";

const errorMessages = {
  "overdrawn balance": {
    message: "buy_modal_msg_fail_overdrawn"
  },
  "no enough eos": {
    message: "buy_modal_msg_fail_no_enough",
    refresh: true
  },
  "pls check amount": {
    message: "buy_modal_msg_fail_no_enough",
    refresh: true
  },
  "game not start": {
    message: "buy_modal_msg_fail_time"
  },
  "game over": {
    message: "buy_modal_msg_fail_time"
  },
  "no character exist": {
    message: "buy_modal_msg_fail_celeb_not_exist"
  }
};

export default {
  name: "BuyModal",
  props: ["priceInfo"],
  methods: {
    getPrice() {
      if (!this.priceInfo) {
        return 0;
      } else {
        return this.priceInfo.nextprice;
      }
    },
    getchecksum(id, price) {
      return id * price + 10086;
    },
    async buy() {
      const { account, eos } = this;
      const price = this.getPrice();
      const priceReadable = `${(price / 10000).toFixed(4)} ALLIN`;
      const buyTarget = this.celebBaseList[this.priceInfo.id].name;
      const memo = [
        "buy",
        String(this.priceInfo.id),
        this.getchecksum(this.priceInfo.id, price)
      ];

      try {
        await eos.transaction({
          actions: [
            {
              account: "eosbocai1111",
              name: "transfer",
              authorization: [
                {
                  actor: account.name,
                  permission: account.authority
                }
              ],
              data: {
                from: account.name,
                to: "eosbocaihero",
                quantity: priceReadable,
                memo: `${memo.join(" ")}`
              }
            }
          ]
        });
        this.$toast.open({
          type: "is-success",
          duration: 5000,
          message: this.$t("buy_modal_msg_success", {
            priceReadable,
            buyTarget
          }),
          position: "is-bottom",
          queue: false
        });
        this.$parent.close();
        this.$store.dispatch("updateCeleb");
      } catch (error) {
        if (typeof error === "object") {
          if (
            error.message.indexOf("User rejected the signature request") > -1
          ) {
            this.$parent.close();
            this.$toast.open({
              type: "is-danger",
              duration: 5000,
              message: this.$t("buy_modal_msg_cancel", { buyTarget }),
              position: "is-bottom",
              queue: false
            });
          } else {
            this.$dialog.alert({
              title: this.$t("buy_modal_msg_fail_title"),
              message: this.$t("buy_modal_msg_fail_body", {
                priceReadable,
                buyTarget,
                content: `Unknown Error: <br>${util.escapeHtml(error.message)}`
              }),
              onConfirm: () => {
                this.$parent.close();
                this.$store.dispatch("updateCeleb");
              }
            });
          }
        } else {
          const errorStr = String(error);
          for (let errorKeyword in errorMessages) {
            // Found a known error
            const errorProc = errorMessages[errorKeyword];
            if (errorStr.indexOf(errorKeyword) > -1) {
              this.$dialog.alert({
                title: this.$t("buy_modal_msg_fail_title"),
                message: this.$t("buy_modal_msg_fail_body", {
                  priceReadable,
                  buyTarget,
                  content: this.$t(errorProc.message)
                }),
                onConfirm: () => {
                  if (errorProc.refresh) {
                    this.$parent.close();
                    this.$store.dispatch("updateCeleb");
                  }
                }
              });
              return;
            }
          }
          // Error: unknown error
          this.$dialog.alert({
            title: this.$t("buy_modal_msg_fail_title"),
            message: this.$t("buy_modal_msg_fail_body", {
              priceReadable,
              buyTarget,
              content: `Unknown Error: <br><pre style="white-space:pre-wrap;word-wrap:break-word;">${util.escapeHtml(
                errorStr
              )}</pre>`
            }),
            onConfirm: () => {
              this.$parent.close();
              this.$store.dispatch("updateCeleb");
            }
          });
        }
      }
    }
  },
  computed: {
    ...mapState(["identity", "scatter", "eos", "account", "celebBaseList"]),
    ...mapGetters(["account"])
  }
};
</script>
