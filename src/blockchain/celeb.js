import {
  eos
} from './store'
import celebList from './mock_celeb.json'
import celebIdList from './mock_celebid.json'
import {
  network
} from '@/config'

const theContractAccount = 'eosbocaihero'

const requiredFields = {
  accounts: [network]
}

const celebIdMap = {}
celebList.forEach(celeb => {
  celebIdMap[celeb.id] = celeb
})
const celebIdContractMap = {}
for (let contractId in celebIdList) {
  celebIdContractMap[contractId] = celebIdMap[celebIdList[contractId]]
}

export async function getGlobal () {
  const {
    rows
  } = await eos().getTableRows({
    json: 'true',
    code: 'eosbocaihero',
    scope: 'eosbocaihero',
    table: 'bagsglobal'
  })
  return rows[0]
}

export async function getCelebBaseList () {
  return celebIdContractMap
}

export async function getCelebPriceList () {
  const {
    rows
  } = await eos().getTableRows({
    json: 'true',
    code: 'eosbocaihero',
    scope: 'eosbocaihero',
    limit: 200,
    table: 'bag'
  })
  return rows
}

export async function changeMySlogan ({
  from,
  id,
  memo,
  authority = 'active'
}) {
  const contract = await eos().contract(theContractAccount, {
    requiredFields
  })
  return contract.setslogan(from, id, memo, {
    authorization: [`${from}@${authority}`]
  })
}
